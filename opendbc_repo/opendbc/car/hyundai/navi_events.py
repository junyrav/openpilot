"""
Stock-navigation road events for HKG CAN-FD cars that broadcast ADASIS-style navigation data on the E-CAN
(2026 Kia K8 HEV PE; same messages CarrotPilot uses for its "vehicle navi" speed control).

  0x4A3 HDA_INFO      SPEED_LIMIT (byte 1), MapSource (bits 27-29; 2 = speed-camera warning active)
  0x4B4 POSITION      POS_RANGE_AVG_SPEED (bits 21-29; 0 < v < 511 inside a section-enforcement zone)
  0x4BE PROFILE LONG  value (bits 0-31), offset m (32-44), path index (48-53), profile type (54-58)
                      type 16: value 6 = speed bump; kind (value & 0xf) 0/1/2 = speed camera,
                      speed code (value >> 4): limit = (code - 1) * 5 km/h

Distances are tracked by integrating vEgo. 30 km/h cameras/zones (school zones) are ignored on purpose.
"""
import os

# Touch this file on the device (and reboot) to turn the stock-navi slowdown off.
DISABLE_FLAG_PATH = "/data/k8_navi_speed_off"

PROFILE_TYPE_SPOT = 16
BUMP_VALUE = 6
CAMERA_KINDS = (0, 1, 2)
MAX_EVENT_DISTANCE = 2500.0   # m, ignore profiles further than this
DEDUPE_DISTANCE = 20.0        # m, same event re-announced with a slightly different offset
CAMERA_PASSED_DISTANCE = 30.0  # m past the announced point before a camera event is retired
BUMP_PASSED_DISTANCE = 10.0   # m past the announced point before a bump event is retired
CAMERA_MATCH_MARGIN = 40.0    # m, announced camera offsets can point 30-40 m beyond the camera
CAMERA_MATCH_WINDOW = 1000.0   # m, the stock navi warns within ~1 km of a camera
CAMERA_FALLBACK_DISTANCE = 350.0  # m, warning without a matching profile: camera assumed this far ahead (logs: 380-530 m)
MIN_ENFORCED_SPEED = 30       # km/h, cameras/zones at or below this (school zones) are ignored
MAX_EVENTS = 32
NO_BUMP = -100.0               # bump_distance when no bump is ahead
POSITION_TIMEOUT = 1.0        # s


def navi_events_enabled() -> bool:
  return not os.path.exists(DISABLE_FLAG_PATH)


def decode_profile(dat: bytes):
  d = int.from_bytes(bytes(dat[:8]), "little")
  return {
    "value": d & 0xFFFFFFFF,
    "offset": (d >> 32) & 0x1FFF,
    "path_index": (d >> 48) & 0x3F,
    "profile_type": (d >> 54) & 0x1F,
  }


def classify_profile(profile):
  """-> (type, speed_kph) or None"""
  if profile["profile_type"] != PROFILE_TYPE_SPOT:
    return None
  if not 0 < profile["offset"] <= MAX_EVENT_DISTANCE:
    return None
  value = profile["value"]
  if value == BUMP_VALUE:
    return "bump", 0
  if not 0 < value <= 0x1FF:
    return None
  kind = value & 0xF
  speed_code = value >> 4
  if kind not in CAMERA_KINDS or not 1 < speed_code <= 31:
    return None
  speed = (speed_code - 1) * 5
  if speed <= MIN_ENFORCED_SPEED:
    return None
  return "camera", speed


class NaviEvents:
  def __init__(self):
    self.total_distance = 0.0
    self.events = []  # dicts: type, speed, target (absolute traveled distance)
    self.speed_limit = 0
    self.camera_active = False
    self.status_event = None
    self.status_speed = 0
    self.fallback_target = None
    self.range_avg_speed = 0
    self.position_age = POSITION_TIMEOUT + 1.0

    # outputs
    self.camera_speed = 0.0     # km/h, 0 = none
    self.camera_distance = 0.0  # m (can be <= 0 while passing)
    self.bump_distance = NO_BUMP  # m
    self.section_speed = 0.0    # km/h, 0 = none

  def _add(self, event_type, speed, offset):
    target = self.total_distance + offset
    for event in self.events:
      if event["type"] == event_type and event["speed"] == speed and abs(event["target"] - target) < DEDUPE_DISTANCE:
        event["target"] = target
        return
    self.events.append({"type": event_type, "speed": speed, "target": target})
    self.events.sort(key=lambda e: e["target"])
    del self.events[MAX_EVENTS:]

  def update(self, v_ego: float, dt: float, profiles: list[bytes], hda_info: bytes | None, position: bytes | None):
    self.total_distance += max(v_ego, 0.0) * dt

    for dat in profiles:
      if len(dat) >= 8:
        event = classify_profile(decode_profile(dat))
        if event is not None:
          self._add(event[0], event[1], decode_profile(dat)["offset"])

    if hda_info is not None and len(hda_info) >= 4:
      limit = hda_info[1]
      self.speed_limit = limit if 0 < limit < 255 else 0
      self.camera_active = ((hda_info[3] >> 3) & 0x7) == 2 and self.speed_limit > MIN_ENFORCED_SPEED

    if position is not None and len(position) >= 4:
      self.range_avg_speed = (int.from_bytes(bytes(position[:4]), "little") >> 21) & 0x1FF
      self.position_age = 0.0
    else:
      self.position_age += dt

    # retire passed events
    self.events = [e for e in self.events if e["target"] >= self.total_distance -
                   (CAMERA_PASSED_DISTANCE if e["type"] == "camera" else BUMP_PASSED_DISTANCE)]

    # current camera warning (0x4A3) -> match it to an announced camera so the target distance is known
    if self.camera_active:
      if self.status_speed != self.speed_limit:
        self.status_speed = self.speed_limit
        self.status_event = None
        self.fallback_target = self.total_distance + CAMERA_FALLBACK_DISTANCE
      if self.status_event is None or self.status_event not in self.events:
        matches = [e for e in self.events if e["type"] == "camera" and e["speed"] == self.speed_limit and
                   self.total_distance - CAMERA_PASSED_DISTANCE <= e["target"] <=
                   self.total_distance + CAMERA_MATCH_WINDOW]
        self.status_event = matches[0] if matches else None
    else:
      if self.status_event is not None and self.status_event in self.events:
        self.events.remove(self.status_event)
      self.status_event = None
      self.status_speed = 0
      self.fallback_target = None

    upcoming = [e for e in self.events if e["target"] > self.total_distance - CAMERA_PASSED_DISTANCE]

    self.camera_speed, self.camera_distance = 0.0, 0.0
    if self.camera_active:
      target = self.status_event["target"] if self.status_event is not None else self.fallback_target
      self.camera_speed = float(self.speed_limit)
      self.camera_distance = target - self.total_distance
    else:
      cameras = [e for e in upcoming if e["type"] == "camera" and e["target"] > self.total_distance]
      if cameras:
        self.camera_speed = float(cameras[0]["speed"])
        self.camera_distance = cameras[0]["target"] - self.total_distance

    bumps = [e for e in self.events if e["type"] == "bump"]
    self.bump_distance = bumps[0]["target"] - self.total_distance if bumps else NO_BUMP

    section = (self.position_age <= POSITION_TIMEOUT and 0 < self.range_avg_speed < 511 and
               self.speed_limit > MIN_ENFORCED_SPEED)
    self.section_speed = float(self.speed_limit) if section else 0.0
