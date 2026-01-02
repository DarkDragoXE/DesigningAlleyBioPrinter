/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2025 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Peltier Bidirectional Temperature Control for Bioprinting
 *
 * Hardware Setup:
 * ┌──────────────────────────────────────────────────────────────┐
 * │ BTT Octopus → ULN2003 → DPDT Relay → Peltier Module         │
 * ├──────────────────────────────────────────────────────────────┤
 * │ PD12 (FAN2) → ULN IN1 → OUT1 → DPDT Coil (polarity control) │
 * │ PA2 (HE0)   → MOSFET → DPDT COM → Peltier (PWM power)       │
 * │ 12V SMPS    → MOSFET VIN                                     │
 * │ 12V SMPS    → DPDT Coil (other end)                          │
 * └──────────────────────────────────────────────────────────────┘
 *
 * Control Logic:
 * - PA2 (HE0): PWM controls power/current to Peltier (0-255)
 * - PD12 (FAN2): Digital HIGH/LOW controls polarity via DPDT:
 *   • PD12 HIGH → DPDT energized → Peltier HEATING mode
 *   • PD12 LOW  → DPDT relaxed   → Peltier COOLING mode
 *
 * Usage:
 *   M104 S37  ; Set target temperature to 37°C (body temp)
 *   M109 S4   ; Set to 4°C and wait (cooling)
 *   M104 S0   ; Turn off Peltier
 */

#pragma once

#include "../inc/MarlinConfig.h"

#if ENABLED(PELTIER_CONTROL_E0)

class PeltierControl {
public:
  enum Mode : uint8_t {
    PELTIER_OFF,      // Both HE0 and PD12 off
    PELTIER_HEATING,  // PD12 HIGH (DPDT energized), HE0 PWM
    PELTIER_COOLING   // PD12 LOW (DPDT relaxed), HE0 PWM
  };

  static Mode current_mode;
  static uint8_t power_pwm;  // Current PWM value (0-255)

  /**
   * Initialize Peltier control pins
   * Called from setup()
   */
  static void init();

  /**
   * Set Peltier mode and power
   * @param mode PELTIER_HEATING, PELTIER_COOLING, or PELTIER_OFF
   * @param pwm Power level 0-255 (ignored if mode is OFF)
   */
  static void set_mode(Mode mode, uint8_t pwm = 0);

  /**
   * Emergency stop - turn off immediately
   */
  static void emergency_stop();

  /**
   * Get current operating mode
   */
  static Mode get_mode() { return current_mode; }

  /**
   * Get current PWM value
   */
  static uint8_t get_pwm() { return power_pwm; }

private:
  /**
   * Apply current mode and PWM to hardware pins
   * Includes safety interlock delay when switching polarity
   */
  static void apply_to_hardware();

  /**
   * Safety: Ensure MOSFET is OFF before changing DPDT polarity
   */
  static void safe_polarity_change(bool heating_mode);
};

extern PeltierControl peltier;

#endif // PELTIER_CONTROL_E0
