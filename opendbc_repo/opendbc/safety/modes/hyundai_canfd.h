#pragma once

// Provenance: portions of HKG angle-command safety are adapted from sunnypilot/opendbc's
// hkg-angle-steering-2025 branch at cc4b08625. See CREDITS.md and THIRD_PARTY_NOTICES.md.
#include "opendbc/safety/declarations.h"
#include "opendbc/safety/modes/hyundai_common.h"

#define HYUNDAI_CANFD_CRUISE_BUTTON_TX_MSGS(bus) \
  {0x1CF, bus, 8, .check_relay = false},  /* CRUISE_BUTTON */   \

#define HYUNDAI_CANFD_ALT_CRUISE_BUTTON_TX_MSGS(bus) \
  {0x1AA, bus, 16, .check_relay = false},  /* CRUISE_BUTTONS_ALT */ \

#define HYUNDAI_CANFD_LKA_STEERING_COMMON_TX_MSGS(a_can, e_can) \
  HYUNDAI_CANFD_CRUISE_BUTTON_TX_MSGS(e_can)                        \
  {0x50,  a_can, 16, .check_relay = (a_can) == 0},  /* LKAS */      \
  {0x2A4, a_can, 24, .check_relay = (a_can) == 0},  /* CAM_0x2A4 */ \

#define HYUNDAI_CANFD_LKA_STEERING_ALT_COMMON_TX_MSGS(a_can, e_can) \
  HYUNDAI_CANFD_CRUISE_BUTTON_TX_MSGS(e_can)                        \
  {0x110, a_can, 32, .check_relay = (a_can) == 0, .disable_static_blocking = true},  /* LKAS_ALT */  \
  {0x362, a_can, 32, .check_relay = (a_can) == 0, .disable_static_blocking = true},  /* CAM_0x362 */ \

#define HYUNDAI_CANFD_LFA_STEERING_COMMON_TX_MSGS(e_can)  \
  {0x12A, e_can, 16, .check_relay = (e_can) == 0},  /* LFA */            \
  {0x1E0, e_can, 16, .check_relay = (e_can) == 0},  /* LFAHDA_CLUSTER */ \
  {0xCB,  e_can, 24, .check_relay = (e_can) == 0},  /* ADAS_CMD_35_10ms */ \

#define HYUNDAI_CANFD_SCC_CONTROL_COMMON_TX_MSGS(e_can, longitudinal) \
  {0x1A0, e_can, 32, .check_relay = (longitudinal)},  /* SCC_CONTROL */ \

#define HYUNDAI_CANFD_MRR35_RADAR_TRACK_START 0x3A5
#define HYUNDAI_CANFD_MRR35_RADAR_TRACK_END 0x3C4
#define HYUNDAI_CANFD_INACTIVE_ACCEL_TX_THRESHOLD 10U

#define HYUNDAI_CANFD_BLINDSPOT_DASH_TX_MSGS(e_can) \
  {0x1BA, e_can, 24, .check_relay = false},  /* BLINDSPOTS_REAR_CORNERS */ \
  {0x1E5, e_can, 16, .check_relay = false},  /* BLINDSPOTS_FRONT_CORNER_1 */ \
  {0x31A, e_can, 32, .check_relay = false},  /* cluster blindspot overlay */ \
  {0x3B5, e_can, 32, .check_relay = false},  /* cluster blindspot overlay */ \
  {0x3C1, e_can, 8, .check_relay = false},  /* cluster lane change overlay */ \

// *** Addresses checked in rx hook ***
// EV, ICE, HYBRID: ACCELERATOR (0x35), ACCELERATOR_BRAKE_ALT (0x100), ACCELERATOR_ALT (0x105)
#define HYUNDAI_CANFD_COMMON_RX_CHECKS(pt_bus)                                                                          \
  {.msg = {{0x35, (pt_bus), 32, 100U, .max_counter = 0xffU, .ignore_quality_flag = true},                  \
           {0x100, (pt_bus), 32, 100U, .max_counter = 0xffU, .ignore_quality_flag = true},                 \
           {0x105, (pt_bus), 32, 100U, .max_counter = 0xffU, .ignore_quality_flag = true}}},               \
  {.msg = {{0x175, (pt_bus), 24, 50U, .max_counter = 0xffU, .ignore_quality_flag = true}, { 0 }, { 0 }}},  \
  {.msg = {{0xa0, (pt_bus), 24, 100U, .max_counter = 0xffU, .ignore_quality_flag = true}, { 0 }, { 0 }}},  \
  {.msg = {{0xea, (pt_bus), 24, 100U, .max_counter = 0xffU, .ignore_quality_flag = true}, { 0 }, { 0 }}},  \

#define HYUNDAI_CANFD_STD_BUTTONS_RX_CHECKS(pt_bus)                                                                                            \
  HYUNDAI_CANFD_COMMON_RX_CHECKS(pt_bus)                                                                                                       \
  {.msg = {{0x1cf, (pt_bus), 8, 50U, .ignore_checksum = true, .max_counter = 0xfU, .ignore_quality_flag = true}, { 0 }, { 0 }}},  \

#define HYUNDAI_CANFD_ALT_BUTTONS_RX_CHECKS(pt_bus)                                                                                              \
  HYUNDAI_CANFD_COMMON_RX_CHECKS(pt_bus)                                                                                                         \
  {.msg = {{0x1aa, (pt_bus), 16, 50U, .ignore_checksum = true, .max_counter = 0xffU, .ignore_quality_flag = true}, { 0 }, { 0 }}},  \

// SCC_CONTROL (from ADAS unit or camera)
#define HYUNDAI_CANFD_SCC_ADDR_CHECK(scc_bus)                                                                            \
  {.msg = {{0x1a0, (scc_bus), 32, 50U, .max_counter = 0xffU, .ignore_quality_flag = true}, { 0 }, { 0 }}},  \

static bool hyundai_canfd_alt_buttons = false;
static bool hyundai_canfd_lka_steering_alt = false;
static bool hyundai_canfd_angle_steering = false;
static bool hyundai_ccnc = false;
static bool hyundai_canfd_ccnc_angle_long = false;
static bool hyundai_canfd_lka_alt_drive_gear = false;
static uint8_t hyundai_canfd_inactive_accel_tx_count = 0U;

static unsigned int hyundai_canfd_get_lka_addr(void) {
  return hyundai_canfd_lka_steering_alt ? 0x110U : 0x50U;
}

static uint8_t hyundai_canfd_get_counter(const CANPacket_t *msg) {
  uint8_t ret = 0;
  if (GET_LEN(msg) == 8U) {
    ret = msg->data[1] >> 4;
  } else {
    ret = msg->data[2];
  }
  return ret;
}

static uint32_t hyundai_canfd_get_checksum(const CANPacket_t *msg) {
  uint32_t chksum = msg->data[0] | (msg->data[1] << 8);
  return chksum;
}

#define HYUNDAI_CANFD_LFA_ANGLE_MAX_TX_MSGS 32

static bool hyundai_canfd_lka_alt_forward_addr(int addr) {
  return (addr == 0x110) || (addr == 0x362);
}

static bool hyundai_canfd_lka_alt_openpilot_allowed(void) {
  const bool angle_steering_allowed = !hyundai_canfd_angle_steering || vehicle_moving;
  return (aol_allowed || controls_allowed) && angle_steering_allowed &&
         (!hyundai_ev_gas_signal || hyundai_canfd_lka_alt_drive_gear);
}

static bool hyundai_canfd_lka_alt_stock_forwarding(void) {
  return hyundai_canfd_lka_steering_alt && hyundai_canfd_angle_steering && !hyundai_canfd_lka_alt_openpilot_allowed();
}

// LFA-path angle trims (angle steering without LKA steering, e.g. 2026 K8 HEV PE via the ADAS/ADRV harness):
// the ADRV's LFA (0x12A) and ADAS_CMD_35 (0xCB) are passed through untouched while openpilot is not in control,
// so the MDPS/cluster keep seeing the exact stock frames (status, counters, FCA-ESA fields). openpilot's own
// frames for these addresses are only accepted, and the stock ones only blocked, while controls are allowed.
static bool hyundai_canfd_lfa_angle_forward_addr(int addr) {
  return (addr == 0x12a) || (addr == 0xcb);
}

static bool hyundai_canfd_lfa_angle_path(void) {
  return hyundai_canfd_angle_steering && !hyundai_canfd_lka_steering;
}

static bool hyundai_canfd_lfa_angle_stock_forwarding(void) {
  return hyundai_canfd_lfa_angle_path() && !(aol_allowed || controls_allowed);
}

// Latest LFA angle-active state (0xCB byte 3 bits 4-5) commanded by the stock ADRV on bus 2; -1 until seen.
static int hyundai_canfd_adrv_lfa_angle_active = -1;
static uint32_t hyundai_canfd_mdps_echo_frames = 0U;

static void hyundai_canfd_rx_all_hook(const CANPacket_t *msg) {
  if (hyundai_canfd_lfa_angle_path() && (msg->bus == 2U) && (msg->addr == 0xcbU) && (GET_LEN(msg) == 24U)) {
    hyundai_canfd_adrv_lfa_angle_active = (int)((msg->data[3] >> 4U) & 0x3U);
  }
}

// LFA-path angle trims: while openpilot owns 0xCB, the MDPS follows openpilot instead of the ADRV, so the MDPS
// angle-active state (0xEA byte 18 bits 0-1) the ADRV receives no longer matches its own 0xCB command. The ADRV
// treats that as a steering fault and latches FAULT_LFA/LCA/FCA (2026 K8 HEV PE rlogs; LKAS press -> steering error).
// Like CarrotPilot, echo the ADRV's own state in the forwarded MDPS frame, in the stock frame's slot, so the stock
// counter and timing are untouched (only the checksum is recomputed). While the stock ADRV drives the MDPS
// (stock forwarding), the real frame is forwarded unchanged.
static void hyundai_canfd_fwd_modify_hook(CANPacket_t *msg) {
  const bool echo = hyundai_canfd_lfa_angle_path() && !hyundai_canfd_lfa_angle_stock_forwarding() &&
                    (hyundai_canfd_adrv_lfa_angle_active >= 0) &&
                    (msg->bus == 0U) && (msg->addr == 0xeaU) && (GET_LEN(msg) == 24U);
  if (echo) {
    msg->data[18] = (uint8_t)((msg->data[18] & 0xFCU) | ((uint32_t)hyundai_canfd_adrv_lfa_angle_active & 0x3U));

    // The ADRV now believes its LFA is steering, so its own hands-on monitor would nag about a system that is not
    // driving the car. Like CarrotPilot, add a small column-torque pulse (+220 raw for 40 of every 1000 frames,
    // i.e. 0.4 s every 10 s) to the copy the ADRV sees only. openpilot's driver monitoring is unaffected, and the
    // panda's own driver-torque checks use the real frame.
    hyundai_canfd_mdps_echo_frames = (hyundai_canfd_mdps_echo_frames + 1U) % 1000U;
    if (hyundai_canfd_mdps_echo_frames < 40U) {
      uint32_t col_torque = ((uint32_t)(msg->data[11] & 0x1FU) << 8U) | (uint32_t)msg->data[10];
      col_torque = SAFETY_MIN(col_torque + 220U, 0x1FFFU);
      msg->data[10] = (uint8_t)(col_torque & 0xFFU);
      msg->data[11] = (uint8_t)((msg->data[11] & 0xE0U) | ((col_torque >> 8U) & 0x1FU));
    }
    uint32_t checksum = hyundai_common_canfd_compute_checksum(msg);
    msg->data[0] = (uint8_t)(checksum & 0xFFU);
    msg->data[1] = (uint8_t)((checksum >> 8U) & 0xFFU);
  }
}

static bool hyundai_canfd_fwd_hook(int bus_num, int addr) {
  const bool mrr35_radar_track = (addr >= HYUNDAI_CANFD_MRR35_RADAR_TRACK_START) && (addr <= HYUNDAI_CANFD_MRR35_RADAR_TRACK_END);

  if ((bus_num == 2) && hyundai_canfd_lka_steering_alt && hyundai_canfd_lka_alt_forward_addr(addr)) {
    return !hyundai_canfd_lka_alt_stock_forwarding();
  }

  if ((bus_num == 2) && hyundai_canfd_lfa_angle_path() && hyundai_canfd_lfa_angle_forward_addr(addr)) {
    return !hyundai_canfd_lfa_angle_stock_forwarding();
  }

  // On LKA-steering long-control cars using live MRR35 radar tracks, openpilot parses
  // the tracks directly from bus 0. Forwarding them to bus 2 creates a returned TX copy
  // of every object frame on the logged CAN stream without adding planner data.
  return hyundai_longitudinal && hyundai_canfd_lka_steering && (bus_num == 0) && mrr35_radar_track;
}

static void hyundai_canfd_rx_hook(const CANPacket_t *msg) {

  const unsigned pt_bus = hyundai_canfd_lka_steering ? 1U : 0U;
  const unsigned int scc_bus = hyundai_camera_scc ? 2U : pt_bus;

  if (msg->bus == pt_bus) {
    // driver torque
    if (msg->addr == 0xeaU) {
      int torque_driver_new = ((msg->data[11] & 0x1fU) << 8U) | msg->data[10];
      torque_driver_new -= 4095;
      update_sample(&torque_driver, torque_driver_new);

      // CCNC angle-long platforms publish the usable angle in STEERING_ANGLE_2.
      const unsigned int angle_offset = hyundai_canfd_ccnc_angle_long ? 16U : 12U;
      int angle_meas_new = (msg->data[angle_offset + 1U] << 8U) | msg->data[angle_offset];
      angle_meas_new = to_signed(angle_meas_new, 16);
      update_sample(&angle_meas, angle_meas_new);
    }

    // cruise buttons
    const unsigned int button_addr = hyundai_canfd_alt_buttons ? 0x1aaU : 0x1cfU;
    if (msg->addr == button_addr) {
      const bool controls_allowed_prev = controls_allowed;
      bool main_button = false;
      int cruise_button = 0;
      if (msg->addr == 0x1cfU) {
        cruise_button = msg->data[2] & 0x7U;
        main_button = GET_BIT(msg, 19U);

        hyundai_lkas_button_check(GET_BIT(msg, 23U));
      } else {
        cruise_button = (msg->data[4] >> 4) & 0x7U;
        main_button = GET_BIT(msg, 34U);

        hyundai_lkas_button_check(GET_BIT(msg, 39U));
      }
      hyundai_common_cruise_buttons_check(cruise_button, main_button);
      if (!controls_allowed_prev && controls_allowed) {
        hyundai_canfd_inactive_accel_tx_count = 0U;
      }
    }

    // gas press, different for EV, hybrid, and ICE models
    if ((msg->addr == 0x35U) && hyundai_ev_gas_signal) {
      gas_pressed = msg->data[5] != 0U;
      hyundai_canfd_lka_alt_drive_gear = (msg->data[24] & 0x7U) == 5U;
    } else if ((msg->addr == 0x105U) && hyundai_hybrid_gas_signal) {
      gas_pressed = GET_BIT(msg, 103U) || (msg->data[13] != 0U) || GET_BIT(msg, 112U);
    } else if ((msg->addr == 0x100U) && !hyundai_ev_gas_signal && !hyundai_hybrid_gas_signal) {
      gas_pressed = GET_BIT(msg, 176U);
    } else {
    }

    // brake press
    if (msg->addr == 0x175U) {
      brake_pressed = GET_BIT(msg, 81U);
    }

    // vehicle moving
    if (msg->addr == 0xa0U) {
      uint32_t fl = (GET_BYTES(msg, 8, 2)) & 0x3FFFU;
      uint32_t fr = (GET_BYTES(msg, 10, 2)) & 0x3FFFU;
      uint32_t rl = (GET_BYTES(msg, 12, 2)) & 0x3FFFU;
      uint32_t rr = (GET_BYTES(msg, 14, 2)) & 0x3FFFU;
      vehicle_moving = (fl > HYUNDAI_STANDSTILL_THRSLD) || (fr > HYUNDAI_STANDSTILL_THRSLD) ||
                       (rl > HYUNDAI_STANDSTILL_THRSLD) || (rr > HYUNDAI_STANDSTILL_THRSLD);

      // average of all 4 wheel speeds. Conversion: raw * 0.03125 / 3.6 = m/s
      UPDATE_VEHICLE_SPEED((fr + rr + rl + fl) / 4.0 * 0.03125 * KPH_TO_MS);
    }
  }

  if (msg->bus == scc_bus) {
    // cruise state
    if ((msg->addr == 0x1a0U) && !hyundai_longitudinal) {
      // 1=enabled, 2=driver override
      int cruise_status = ((msg->data[8] >> 4) & 0x7U);
      bool cruise_engaged = (cruise_status == 1) || (cruise_status == 2);
      hyundai_common_cruise_state_check(cruise_engaged);
      acc_main_on = GET_BIT(msg, 66U);
    }
  }

  hyundai_common_reset_acc_main_on_mismatches();
}

static bool hyundai_canfd_tx_hook(const CANPacket_t *msg) {
  const TorqueSteeringLimits HYUNDAI_CANFD_STEERING_LIMITS = {
    .max_torque = 409,
    .max_rt_delta = 375,
    .max_rate_up = 10,
    .max_rate_down = 10,
    .driver_torque_allowance = 250,
    .driver_torque_multiplier = 2,
    .type = TorqueDriverLimited,

    // the EPS faults when the steering angle is above a certain threshold for too long. to prevent this,
    // we allow setting torque actuation bit to 0 while maintaining the requested torque value for two consecutive frames
    .min_valid_request_frames = 89,
    .max_invalid_request_frames = 2,
    .min_valid_request_rt_interval = 810000,  // 810ms; a ~10% buffer on cutting every 90 frames
    .has_steer_req_tolerance = true,
  };
  const AngleSteeringLimits HYUNDAI_CANFD_ANGLE_STEERING_LIMITS = {
    .max_angle = 3600,
    .angle_deg_to_can = 10,
    .frequency = 100U,
  };
  const AngleSteeringParams HYUNDAI_CANFD_ANGLE_STEERING_PARAMS = {
    .slip_factor = -0.0006085930193026732,
    .steer_ratio = 13.7,
    .wheelbase = 2.756,
  };

  bool tx = true;

  if ((msg->bus == 0U) && hyundai_canfd_lka_alt_forward_addr(msg->addr) && hyundai_canfd_lka_alt_stock_forwarding()) {
    tx = false;
  }

  if ((msg->bus == 0U) && hyundai_canfd_lfa_angle_forward_addr(msg->addr) && hyundai_canfd_lfa_angle_stock_forwarding()) {
    tx = false;
  }

  if (msg->addr == 0xCBU) {
    if (!hyundai_canfd_angle_steering) {
      tx = false;
    } else {
      const int lfa_angle_active = (msg->data[3] >> 4U) & 0xFU;
      const bool steer_angle_req = lfa_angle_active == 2;

      if (steer_angle_req && hyundai_canfd_ccnc_angle_long && !hyundai_canfd_lka_alt_openpilot_allowed()) {
        tx = false;
      }

      int desired_angle = (((uint32_t)(msg->data[5] & 0x3FU)) << 8) | (uint32_t)msg->data[4];
      desired_angle = to_signed(desired_angle, 14);

      if (steer_angle_cmd_checks_vm(desired_angle, steer_angle_req,
                                    HYUNDAI_CANFD_ANGLE_STEERING_LIMITS,
                                    HYUNDAI_CANFD_ANGLE_STEERING_PARAMS)) {
        tx = false;
      }
    }
  }

  // steering
  const unsigned int steer_addr = (hyundai_canfd_lka_steering && (hyundai_canfd_angle_steering || !hyundai_longitudinal)) ?
                                  hyundai_canfd_get_lka_addr() : 0x12aU;
  if (msg->addr == steer_addr) {
    if (hyundai_canfd_angle_steering) {
      const int lkas_angle_active = (msg->data[9] >> 4U) & 0x3U;
      const bool steer_angle_req = lkas_angle_active != 1;

      int desired_angle = (msg->data[11] << 6U) | (msg->data[10] >> 2U);
      desired_angle = to_signed(desired_angle, 14);

      // ADAS_ACIAnglTqRedcGainVal: bit 96, 8 bits, unsigned. Raw 0-250 valid, 251-255 reserved.
      const uint8_t gain_raw = msg->data[12];

      // LFA-steering angle trims (e.g. 2026 K8 HEV PE) actuate through ADAS_CMD_35 (0xCB) and only need the
      // stock-style LFA frame as a status companion. That companion carries no actuation at all: angle-active
      // field 0, zero angle, zero gain, zero torque and no steer request. Accept it without running it through
      // the angle checks, otherwise it double-counts against the 0xCB real-time rate limit and resets the
      // desired-angle tracking. Anything that carries actuation still goes through the full checks below.
      const int lfa_torque = (((msg->data[6] & 0xFU) << 7U) | (msg->data[5] >> 1U)) - 1024U;
      const bool lfa_steer_req = GET_BIT(msg, 52U);
      // Raw torque 1024 (=0) is what openpilot packs, raw 0 is what the stock ADRV sends; neither is a request.
      const bool lfa_torque_idle = (lfa_torque == 0) || (lfa_torque == -1024);
      // On these trims bytes 10-12 of the LFA frame hold lane/status info (2026 K8 HEV PE rlog: 3 distinct stock
      // payloads while the wheel moved -65..175 deg), so an angle-active field of 0 with no steer request and an
      // idle torque field marks the frame as status-only regardless of those bytes.
      const bool lfa_status_companion = !hyundai_canfd_lka_steering && (steer_addr == 0x12aU) &&
                                        (lkas_angle_active == 0) && lfa_torque_idle && !lfa_steer_req;

      // Angle-steering platforms never actuate through the LFA torque fields.
      if ((steer_addr == 0x12aU) && (!lfa_torque_idle || lfa_steer_req)) {
        tx = false;
      }

      if (!lfa_status_companion) {
        bool gain_violation = gain_raw > 250U;
        if (!steer_angle_req && (gain_raw != 0U)) {
          gain_violation = true;
        }

        if (steer_angle_cmd_checks_vm(desired_angle, steer_angle_req,
                                      HYUNDAI_CANFD_ANGLE_STEERING_LIMITS,
                                      HYUNDAI_CANFD_ANGLE_STEERING_PARAMS) || gain_violation) {
          tx = false;
        }
      }
    } else {
      int desired_torque = (((msg->data[6] & 0xFU) << 7U) | (msg->data[5] >> 1U)) - 1024U;
      bool steer_req = GET_BIT(msg, 52U);

      if (steer_torque_cmd_checks(desired_torque, steer_req, HYUNDAI_CANFD_STEERING_LIMITS)) {
        tx = false;
      }
    }
  }

  // cruise buttons check
  if ((msg->addr == 0x1cfU) || (hyundai_canfd_alt_buttons && (msg->addr == 0x1aaU))) {
    int button = (msg->addr == 0x1aaU) ? ((msg->data[4] >> 4U) & 0x7U) : (msg->data[2] & 0x7U);
    bool is_cancel = (button == HYUNDAI_BTN_CANCEL);
    bool is_resume = (button == HYUNDAI_BTN_RESUME);
    bool is_set = (button == HYUNDAI_BTN_SET);

    bool allowed = (is_cancel && cruise_engaged_prev) || ((is_resume || is_set) && controls_allowed);
    if (!allowed) {
      tx = false;
    }
  }

  // UDS: only tester present ("\x02\x3E\x80\x00\x00\x00\x00\x00") allowed on diagnostics address
  if (((msg->addr == 0x730U) && hyundai_canfd_lka_steering) || ((msg->addr == 0x7D0U) && !hyundai_camera_scc)) {
    if ((GET_BYTES(msg, 0, 4) != 0x00803E02U) || (GET_BYTES(msg, 4, 4) != 0x0U)) {
      tx = false;
    }
  }

  // ACCEL: safety check
  if (msg->addr == 0x1a0U) {
    int desired_accel_raw = (((msg->data[17] & 0x7U) << 8) | msg->data[16]) - 1023U;
    int desired_accel_val = ((msg->data[18] << 4) | (msg->data[17] >> 4)) - 1023U;

    bool violation = false;

    if (hyundai_longitudinal) {
      const int acc_mode = (msg->data[8] >> 4) & 0x7U;
      const bool inactive_accel = (acc_mode == 0) && (desired_accel_raw == 0) && (desired_accel_val == 0);
      if (inactive_accel) {
        hyundai_canfd_inactive_accel_tx_count = SAFETY_MIN(hyundai_canfd_inactive_accel_tx_count + 1U,
                                                           HYUNDAI_CANFD_INACTIVE_ACCEL_TX_THRESHOLD);
        if (hyundai_canfd_inactive_accel_tx_count >= HYUNDAI_CANFD_INACTIVE_ACCEL_TX_THRESHOLD) {
          controls_allowed = false;
        }
      } else {
        hyundai_canfd_inactive_accel_tx_count = 0U;
      }

      violation |= longitudinal_accel_checks(desired_accel_raw, HYUNDAI_LONG_LIMITS);
      violation |= longitudinal_accel_checks(desired_accel_val, HYUNDAI_LONG_LIMITS);
    } else {
      // only used to cancel on here
      const int acc_mode = (msg->data[8] >> 4) & 0x7U;
      if (acc_mode != 4) {
        violation = true;
      }

      if ((desired_accel_raw != 0) || (desired_accel_val != 0)) {
        violation = true;
      }
    }

    if (violation) {
      tx = false;
    }

    acc_main_on_tx = GET_BIT(msg, 66U);
    hyundai_common_acc_main_on_sync();
  }

  return tx;
}

static safety_config hyundai_canfd_init(uint16_t param) {
  const uint16_t HYUNDAI_PARAM_CANFD_LKA_STEERING_ALT = 128;
  const uint16_t HYUNDAI_PARAM_CANFD_ALT_BUTTONS = 32;
  const uint16_t HYUNDAI_PARAM_CANFD_ANGLE_STEERING = 1024;
  const uint16_t HYUNDAI_PARAM_CCNC = 32768U;

  static const CanMsg HYUNDAI_CANFD_LKA_STEERING_TX_MSGS[] = {
    HYUNDAI_CANFD_LKA_STEERING_COMMON_TX_MSGS(0, 1)
  };

  static const CanMsg HYUNDAI_CANFD_LKA_STEERING_ALT_TX_MSGS[] = {
    HYUNDAI_CANFD_LKA_STEERING_ALT_COMMON_TX_MSGS(0, 1)
  };

  static const CanMsg HYUNDAI_CANFD_LKA_STEERING_ALT_BUTTONS_TX_MSGS[] = {
    HYUNDAI_CANFD_LKA_STEERING_COMMON_TX_MSGS(0, 1)
    HYUNDAI_CANFD_SCC_CONTROL_COMMON_TX_MSGS(1, false)
  };

  static const CanMsg HYUNDAI_CANFD_LKA_STEERING_ALT_ALT_BUTTONS_TX_MSGS[] = {
    HYUNDAI_CANFD_LKA_STEERING_ALT_COMMON_TX_MSGS(0, 1)
    HYUNDAI_CANFD_SCC_CONTROL_COMMON_TX_MSGS(1, false)
  };

  static const CanMsg HYUNDAI_CANFD_LKA_STEERING_LONG_TX_MSGS[] = {
    HYUNDAI_CANFD_LKA_STEERING_COMMON_TX_MSGS(0, 1)
    HYUNDAI_CANFD_LFA_STEERING_COMMON_TX_MSGS(1)
    HYUNDAI_CANFD_SCC_CONTROL_COMMON_TX_MSGS(1, true)
    HYUNDAI_CANFD_BLINDSPOT_DASH_TX_MSGS(1)
    {0x51,  0, 32, .check_relay = false},  // ADRV_0x51
    {0x100, 0, 24, .check_relay = false},  // Ioniq 5/6: ACCELERATOR_BRAKE_ALT radar heartbeat spoof
    {0x730, 1,  8, .check_relay = false},  // tester present for ADAS ECU disable
    {0x160, 1, 16, .check_relay = false},  // ADRV_0x160
    {0x1EA, 1, 32, .check_relay = false},  // ADRV_0x1ea
    {0x200, 1,  8, .check_relay = false},  // ADRV_0x200
    {0x345, 1,  8, .check_relay = false},  // ADRV_0x345
    {0x1DA, 1, 32, .check_relay = false},  // ADRV_0x1da
  };

  static const CanMsg HYUNDAI_CANFD_LKA_STEERING_ALT_LONG_TX_MSGS[] = {
    HYUNDAI_CANFD_LKA_STEERING_ALT_COMMON_TX_MSGS(0, 1)
    HYUNDAI_CANFD_LFA_STEERING_COMMON_TX_MSGS(1)
    HYUNDAI_CANFD_SCC_CONTROL_COMMON_TX_MSGS(1, true)
    HYUNDAI_CANFD_BLINDSPOT_DASH_TX_MSGS(1)
    {0x51,  0, 32, .check_relay = false},  // ADRV_0x51
    {0x100, 0, 24, .check_relay = false},  // ACCELERATOR_BRAKE_ALT radar heartbeat spoof
    {0x730, 1,  8, .check_relay = false},  // tester present for ADAS ECU disable
    {0x160, 1, 16, .check_relay = false},  // ADRV_0x160
    {0x1EA, 1, 32, .check_relay = false},  // ADRV_0x1ea
    {0x200, 1,  8, .check_relay = false},  // ADRV_0x200
    {0x345, 1,  8, .check_relay = false},  // ADRV_0x345
    {0x1DA, 1, 32, .check_relay = false},  // ADRV_0x1da
  };

  static const CanMsg HYUNDAI_CANFD_CCNC_ANGLE_LONG_TX_MSGS[] = {
    HYUNDAI_CANFD_LKA_STEERING_ALT_COMMON_TX_MSGS(0, 1)
    HYUNDAI_CANFD_LFA_STEERING_COMMON_TX_MSGS(1)
    HYUNDAI_CANFD_SCC_CONTROL_COMMON_TX_MSGS(1, true)
    {0x1BA, 1, 24, .check_relay = false},  // BLINDSPOTS_REAR_CORNERS
    {0x1E5, 1, 16, .check_relay = false},  // BLINDSPOTS_FRONT_CORNER_1
    {0x100, 0, 24, .check_relay = false},  // ACCELERATOR_BRAKE_ALT radar heartbeat
    {0x730, 1,  8, .check_relay = false},  // tester present for ADAS ECU disable
    {0x160, 1, 16, .check_relay = false},  // ADRV_0x160
    {0x161, 1, 32, .check_relay = false},  // CCNC_0x161
    {0x162, 1, 32, .check_relay = false},  // CCNC_0x162
    {0x1EA, 1, 32, .check_relay = false},  // ADRV_0x1ea
    {0x200, 1,  8, .check_relay = false},  // ADRV_0x200
    {0x345, 1,  8, .check_relay = false},  // ADRV_0x345
    {0x38C, 1, 32, .check_relay = false},  // CCNC support frame
    {0x1DA, 1, 32, .check_relay = false},  // ADRV_0x1da
  };

  static const CanMsg HYUNDAI_CANFD_LFA_STEERING_TX_MSGS[] = {
    HYUNDAI_CANFD_CRUISE_BUTTON_TX_MSGS(2)
    HYUNDAI_CANFD_LFA_STEERING_COMMON_TX_MSGS(0)
    HYUNDAI_CANFD_SCC_CONTROL_COMMON_TX_MSGS(0, false)
  };

  // ADRV_0x160 is checked for radar liveness
  static const CanMsg HYUNDAI_CANFD_LFA_STEERING_LONG_TX_MSGS[] = {
    HYUNDAI_CANFD_CRUISE_BUTTON_TX_MSGS(2)
    HYUNDAI_CANFD_LFA_STEERING_COMMON_TX_MSGS(0)
    HYUNDAI_CANFD_SCC_CONTROL_COMMON_TX_MSGS(0, true)
    HYUNDAI_CANFD_BLINDSPOT_DASH_TX_MSGS(0)
    {0x160, 0, 16, .check_relay = true}, // ADRV_0x160
    {0x7D0, 0, 8, .check_relay = false},  // tester present for radar ECU disable
  };

  // ADRV_0x160 is checked for relay malfunction
#define HYUNDAI_CANFD_LFA_STEERING_CAMERA_SCC_TX_MSGS(longitudinal) \
    HYUNDAI_CANFD_CRUISE_BUTTON_TX_MSGS(2) \
    HYUNDAI_CANFD_LFA_STEERING_COMMON_TX_MSGS(0) \
    HYUNDAI_CANFD_SCC_CONTROL_COMMON_TX_MSGS(0, (longitudinal)) \
    {0x160, 0, 16, .check_relay = (longitudinal)}, /* ADRV_0x160 */ \

#define HYUNDAI_CANFD_LFA_STEERING_CAMERA_SCC_ALT_BUTTONS_TX_MSGS(longitudinal) \
    HYUNDAI_CANFD_ALT_CRUISE_BUTTON_TX_MSGS(2) \
    HYUNDAI_CANFD_LFA_STEERING_COMMON_TX_MSGS(0) \
    HYUNDAI_CANFD_SCC_CONTROL_COMMON_TX_MSGS(0, (longitudinal)) \
    {0x160, 0, 16, .check_relay = (longitudinal)}, /* ADRV_0x160 */ \

#define HYUNDAI_CANFD_LFA_STEERING_CAMERA_SCC_CCNC_TX_MSGS(longitudinal) \
    HYUNDAI_CANFD_CRUISE_BUTTON_TX_MSGS(2) \
    HYUNDAI_CANFD_LFA_STEERING_COMMON_TX_MSGS(0) \
    HYUNDAI_CANFD_SCC_CONTROL_COMMON_TX_MSGS(0, (longitudinal)) \
    {0x161, 0, 32, .check_relay = true}, /* CCNC_0x161 */ \
    {0x162, 0, 32, .check_relay = true}, /* CCNC_0x162 */ \
    {0x7C4, 2, 8, .check_relay = true},  /* camera support frame */ \
    {0xEA, 2, 24, .check_relay = true},  /* MDPS support frame */ \

#define HYUNDAI_CANFD_LFA_STEERING_CAMERA_SCC_CCNC_ALT_BUTTONS_TX_MSGS(longitudinal) \
    HYUNDAI_CANFD_ALT_CRUISE_BUTTON_TX_MSGS(2) \
    HYUNDAI_CANFD_LFA_STEERING_COMMON_TX_MSGS(0) \
    HYUNDAI_CANFD_SCC_CONTROL_COMMON_TX_MSGS(0, (longitudinal)) \
    {0x161, 0, 32, .check_relay = true}, /* CCNC_0x161 */ \
    {0x162, 0, 32, .check_relay = true}, /* CCNC_0x162 */ \
    {0x7C4, 2, 8, .check_relay = true},  /* camera support frame */ \
    {0xEA, 2, 24, .check_relay = true},  /* MDPS support frame */ \

  hyundai_common_init(param);
  hyundai_canfd_adrv_lfa_angle_active = -1;
  hyundai_canfd_mdps_echo_frames = 0U;

  gen_crc_lookup_table_16(0x1021, hyundai_canfd_crc_lut);
  hyundai_canfd_alt_buttons = GET_FLAG(param, HYUNDAI_PARAM_CANFD_ALT_BUTTONS);
  hyundai_canfd_lka_steering_alt = GET_FLAG(param, HYUNDAI_PARAM_CANFD_LKA_STEERING_ALT);
  hyundai_canfd_angle_steering = GET_FLAG(param, HYUNDAI_PARAM_CANFD_ANGLE_STEERING);
  hyundai_ccnc = GET_FLAG(param, HYUNDAI_PARAM_CCNC);
  hyundai_canfd_ccnc_angle_long = hyundai_longitudinal && hyundai_canfd_lka_steering &&
                                  hyundai_canfd_lka_steering_alt && hyundai_canfd_angle_steering && hyundai_ccnc;
  hyundai_canfd_lka_alt_drive_gear = false;
  hyundai_canfd_inactive_accel_tx_count = 0U;

  safety_config ret;
  if (hyundai_longitudinal) {
    if (hyundai_canfd_lka_steering) {
      static RxCheck hyundai_canfd_lka_steering_long_rx_checks[] = {
        HYUNDAI_CANFD_STD_BUTTONS_RX_CHECKS(1)
      };

      static RxCheck hyundai_canfd_lka_steering_alt_buttons_long_rx_checks[] = {
        HYUNDAI_CANFD_ALT_BUTTONS_RX_CHECKS(1)
      };

      if (hyundai_canfd_alt_buttons) {
        SET_RX_CHECKS(hyundai_canfd_lka_steering_alt_buttons_long_rx_checks, ret);
      } else {
        SET_RX_CHECKS(hyundai_canfd_lka_steering_long_rx_checks, ret);
      }
      if (hyundai_canfd_ccnc_angle_long) {
        SET_TX_MSGS(HYUNDAI_CANFD_CCNC_ANGLE_LONG_TX_MSGS, ret);
      } else if (hyundai_canfd_lka_steering_alt) {
        SET_TX_MSGS(HYUNDAI_CANFD_LKA_STEERING_ALT_LONG_TX_MSGS, ret);
      } else {
        SET_TX_MSGS(HYUNDAI_CANFD_LKA_STEERING_LONG_TX_MSGS, ret);
      }

    } else {
      // Longitudinal checks for LFA steering
      static RxCheck hyundai_canfd_long_rx_checks[] = {
        HYUNDAI_CANFD_STD_BUTTONS_RX_CHECKS(0)
      };

      static RxCheck hyundai_canfd_alt_buttons_long_rx_checks[] = {
        HYUNDAI_CANFD_ALT_BUTTONS_RX_CHECKS(0)
      };

      static CanMsg hyundai_canfd_lfa_steering_camera_scc_tx_msgs[] = {
        HYUNDAI_CANFD_LFA_STEERING_CAMERA_SCC_TX_MSGS(true)
        HYUNDAI_CANFD_BLINDSPOT_DASH_TX_MSGS(0)
      };

      static CanMsg hyundai_canfd_lfa_steering_camera_scc_ccnc_tx_msgs[] = {
        HYUNDAI_CANFD_LFA_STEERING_CAMERA_SCC_CCNC_TX_MSGS(true)
      };

      if (hyundai_canfd_alt_buttons) {
        SET_RX_CHECKS(hyundai_canfd_alt_buttons_long_rx_checks, ret);
      } else {
        SET_RX_CHECKS(hyundai_canfd_long_rx_checks, ret);
      }

      if (hyundai_camera_scc) {
        if (hyundai_ccnc) {
          if (hyundai_canfd_alt_buttons) {
            static CanMsg hyundai_canfd_lfa_steering_camera_scc_ccnc_alt_buttons_tx_msgs[] = {
              HYUNDAI_CANFD_LFA_STEERING_CAMERA_SCC_CCNC_ALT_BUTTONS_TX_MSGS(true)
            };
            SET_TX_MSGS(hyundai_canfd_lfa_steering_camera_scc_ccnc_alt_buttons_tx_msgs, ret);
          } else {
            SET_TX_MSGS(hyundai_canfd_lfa_steering_camera_scc_ccnc_tx_msgs, ret);
          }
        } else if (hyundai_canfd_alt_buttons) {
          static CanMsg hyundai_canfd_lfa_steering_camera_scc_alt_buttons_tx_msgs[] = {
            HYUNDAI_CANFD_LFA_STEERING_CAMERA_SCC_ALT_BUTTONS_TX_MSGS(true)
          };
          SET_TX_MSGS(hyundai_canfd_lfa_steering_camera_scc_alt_buttons_tx_msgs, ret);
        } else {
          SET_TX_MSGS(hyundai_canfd_lfa_steering_camera_scc_tx_msgs, ret);
        }
      } else {
        SET_TX_MSGS(HYUNDAI_CANFD_LFA_STEERING_LONG_TX_MSGS, ret);
      }
    }

  } else {
    if (hyundai_canfd_lka_steering) {
      // *** LKA steering checks ***
      // E-CAN is on bus 1, SCC messages are sent on cars with ADRV ECU.
      static RxCheck hyundai_canfd_lka_steering_rx_checks[] = {
        HYUNDAI_CANFD_STD_BUTTONS_RX_CHECKS(1)
        HYUNDAI_CANFD_SCC_ADDR_CHECK(1)
      };

      static RxCheck hyundai_canfd_lka_steering_alt_buttons_rx_checks[] = {
        HYUNDAI_CANFD_ALT_BUTTONS_RX_CHECKS(1)
        HYUNDAI_CANFD_SCC_ADDR_CHECK(1)
      };

      if (hyundai_canfd_alt_buttons) {
        SET_RX_CHECKS(hyundai_canfd_lka_steering_alt_buttons_rx_checks, ret);
      } else {
        SET_RX_CHECKS(hyundai_canfd_lka_steering_rx_checks, ret);
      }
      if (hyundai_canfd_lka_steering_alt) {
        if (hyundai_canfd_alt_buttons) {
          SET_TX_MSGS(HYUNDAI_CANFD_LKA_STEERING_ALT_ALT_BUTTONS_TX_MSGS, ret);
        } else {
          SET_TX_MSGS(HYUNDAI_CANFD_LKA_STEERING_ALT_TX_MSGS, ret);
        }
      } else {
        if (hyundai_canfd_alt_buttons) {
          SET_TX_MSGS(HYUNDAI_CANFD_LKA_STEERING_ALT_BUTTONS_TX_MSGS, ret);
        } else {
          SET_TX_MSGS(HYUNDAI_CANFD_LKA_STEERING_TX_MSGS, ret);
        }
      }

    } else if (!hyundai_camera_scc) {
      // Radar sends SCC messages on these cars instead of camera
      static RxCheck hyundai_canfd_radar_scc_rx_checks[] = {
        HYUNDAI_CANFD_STD_BUTTONS_RX_CHECKS(0)
        HYUNDAI_CANFD_SCC_ADDR_CHECK(0)
      };

      static RxCheck hyundai_canfd_alt_buttons_radar_scc_rx_checks[] = {
        HYUNDAI_CANFD_ALT_BUTTONS_RX_CHECKS(0)
        HYUNDAI_CANFD_SCC_ADDR_CHECK(0)
      };

      SET_TX_MSGS(HYUNDAI_CANFD_LFA_STEERING_TX_MSGS, ret);

      if (hyundai_canfd_alt_buttons) {
        SET_RX_CHECKS(hyundai_canfd_alt_buttons_radar_scc_rx_checks, ret);
      } else {
        SET_RX_CHECKS(hyundai_canfd_radar_scc_rx_checks, ret);
      }

    } else {
      // *** LFA steering checks ***
      // Camera sends SCC messages on LFA steering cars.
      // Both button messages exist on some platforms, so we ensure we track the correct one using flag
      static RxCheck hyundai_canfd_rx_checks[] = {
        HYUNDAI_CANFD_STD_BUTTONS_RX_CHECKS(0)
        HYUNDAI_CANFD_SCC_ADDR_CHECK(2)
      };

      static RxCheck hyundai_canfd_alt_buttons_rx_checks[] = {
        HYUNDAI_CANFD_ALT_BUTTONS_RX_CHECKS(0)
        HYUNDAI_CANFD_SCC_ADDR_CHECK(2)
      };

      static CanMsg hyundai_canfd_lfa_steering_camera_scc_tx_msgs[] = {
        HYUNDAI_CANFD_LFA_STEERING_CAMERA_SCC_TX_MSGS(false)
      };

      static CanMsg hyundai_canfd_lfa_steering_camera_scc_ccnc_tx_msgs[] = {
        HYUNDAI_CANFD_LFA_STEERING_CAMERA_SCC_CCNC_TX_MSGS(false)
      };

      static CanMsg hyundai_canfd_lfa_steering_camera_scc_alt_buttons_tx_msgs[] = {
        HYUNDAI_CANFD_LFA_STEERING_CAMERA_SCC_ALT_BUTTONS_TX_MSGS(false)
      };

      static CanMsg hyundai_canfd_lfa_steering_camera_scc_ccnc_alt_buttons_tx_msgs[] = {
        HYUNDAI_CANFD_LFA_STEERING_CAMERA_SCC_CCNC_ALT_BUTTONS_TX_MSGS(false)
      };

      if (hyundai_ccnc) {
        if (hyundai_canfd_alt_buttons) {
          SET_TX_MSGS(hyundai_canfd_lfa_steering_camera_scc_ccnc_alt_buttons_tx_msgs, ret);
        } else {
          SET_TX_MSGS(hyundai_canfd_lfa_steering_camera_scc_ccnc_tx_msgs, ret);
        }
      } else if (hyundai_canfd_alt_buttons) {
        SET_TX_MSGS(hyundai_canfd_lfa_steering_camera_scc_alt_buttons_tx_msgs, ret);
      } else {
        SET_TX_MSGS(hyundai_canfd_lfa_steering_camera_scc_tx_msgs, ret);
      }

      if (hyundai_canfd_alt_buttons) {
        SET_RX_CHECKS(hyundai_canfd_alt_buttons_rx_checks, ret);
      } else {
        SET_RX_CHECKS(hyundai_canfd_rx_checks, ret);
      }
    }
  }

  // LFA-path angle trims: hand 0x12A/0xCB forwarding to the fwd hook (stock pass-through while disengaged).
  if (hyundai_canfd_lfa_angle_path() && (ret.tx_msgs != NULL) && (ret.tx_msgs_len <= HYUNDAI_CANFD_LFA_ANGLE_MAX_TX_MSGS)) {
    static CanMsg hyundai_canfd_lfa_angle_tx_msgs[HYUNDAI_CANFD_LFA_ANGLE_MAX_TX_MSGS];
    for (int i = 0; i < ret.tx_msgs_len; i++) {
      hyundai_canfd_lfa_angle_tx_msgs[i] = ret.tx_msgs[i];
      if ((hyundai_canfd_lfa_angle_tx_msgs[i].bus == 0U) && hyundai_canfd_lfa_angle_forward_addr(hyundai_canfd_lfa_angle_tx_msgs[i].addr)) {
        hyundai_canfd_lfa_angle_tx_msgs[i].disable_static_blocking = true;
      }
      // The ccNC lists reserve MDPS (0xEA) and the camera diagnostic address (0x7C4) on bus 2 for platforms where
      // openpilot spoofs them to the ADRV. LFA-path trims do not: blocking the real frames starves the ADRV of the
      // MDPS status, and ~6 s later it latches FCA/LCA/DAS faults (2026 K8 HEV PE rlogs). Keep forwarding them.
      if ((hyundai_canfd_lfa_angle_tx_msgs[i].bus == 2U) &&
          ((hyundai_canfd_lfa_angle_tx_msgs[i].addr == 0xEA) || (hyundai_canfd_lfa_angle_tx_msgs[i].addr == 0x7C4))) {
        hyundai_canfd_lfa_angle_tx_msgs[i].disable_static_blocking = true;
      }
    }
    ret.tx_msgs = hyundai_canfd_lfa_angle_tx_msgs;
  }

  return ret;
}

const safety_hooks hyundai_canfd_hooks = {
  .init = hyundai_canfd_init,
  .rx_all = hyundai_canfd_rx_all_hook,
  .rx = hyundai_canfd_rx_hook,
  .tx = hyundai_canfd_tx_hook,
  .get_counter = hyundai_canfd_get_counter,
  .get_checksum = hyundai_canfd_get_checksum,
  .compute_checksum = hyundai_common_canfd_compute_checksum,
  .fwd = hyundai_canfd_fwd_hook,
  .fwd_modify = hyundai_canfd_fwd_modify_hook,
};
