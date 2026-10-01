from opendbc.car.hyundai.navi_events import NaviEvents, NO_BUMP, CAMERA_FALLBACK_DISTANCE


def profile(value, offset, profile_type=16, path=8):
  d = value | (offset << 32) | (path << 48) | (profile_type << 54)
  return d.to_bytes(8, "little")


def hda_info(limit, camera):
  # byte 1 = SPEED_LIMIT, bits 27-29 = MapSource (2 = camera warning)
  return bytes([0x45, limit, 0x9a, 0x11 if camera else 0x09, 0, 0, 1, 0])


def drive(n, ev, v=20.0, **kw):
  for _ in range(n):
    ev.update(v, 0.01, [], kw.get("hda"), None)


def test_bump_tracked_and_retired():
  ev = NaviEvents()
  ev.update(20.0, 0.01, [profile(6, 300)] * 3, None, None)
  assert abs(ev.bump_distance - 300) < 1
  drive(1000, ev)  # 200 m
  assert abs(ev.bump_distance - 100) < 2
  drive(600, ev)   # 120 m more -> 20 m past
  assert ev.bump_distance == NO_BUMP


def test_camera_matched_to_warning_and_released():
  ev = NaviEvents()
  ev.update(20.0, 0.01, [profile((15 << 4) | 1, 569)], hda_info(0, False), None)  # 70 km/h camera, 569 m
  assert ev.camera_speed == 70 and abs(ev.camera_distance - 569) < 1
  drive(150, ev, hda=hda_info(70, True))  # warning on
  assert ev.camera_speed == 70 and abs(ev.camera_distance - 539) < 2
  drive(10, ev, hda=hda_info(0, False))   # warning ends -> event retired
  assert ev.camera_speed == 0


def test_warning_without_profile_uses_fallback():
  ev = NaviEvents()
  ev.update(20.0, 0.01, [], hda_info(60, True), None)
  assert ev.camera_speed == 60 and abs(ev.camera_distance - CAMERA_FALLBACK_DISTANCE) < 1


def test_school_zone_speeds_ignored():
  ev = NaviEvents()
  ev.update(10.0, 0.01, [profile((7 << 4) | 0, 200)], hda_info(30, True), None)  # 30 km/h camera + warning
  assert ev.camera_speed == 0
