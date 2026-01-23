/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2025 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Peltier Bidirectional Temperature Control for Bioprinting
 *
 * Hardware Setup (3 Peltier modules):
 * ┌──────────────────────────────────────────────────────────────────────┐
 * │ PELTIER E0 (Extruder 0):                                            │
 * │   PD12 (FAN2/P60) → ULN2003 → DPDT Coil (polarity control)          │
 * │   PA2 (HE0)       → MOSFET → Peltier (PWM power)                    │
 * ├──────────────────────────────────────────────────────────────────────┤
 * │ PELTIER E1 (Extruder 1):                                            │
 * │   PD13 (FAN3/P61) → ULN2003 → DPDT Coil (polarity control)          │
 * │   PA3 (HE1)       → MOSFET → Peltier (PWM power)                    │
 * ├──────────────────────────────────────────────────────────────────────┤
 * │ PELTIER BED:                                                         │
 * │   PD14 (FAN4/P62) → ULN2003 → DPDT Coil (polarity control)          │
 * │   PB10 (HE2)      → MOSFET → Peltier (PWM power)                    │
 * └──────────────────────────────────────────────────────────────────────┘
 *
 * Control Logic (same for all 3 Peltiers):
 * - HE pin: PWM controls power/current to Peltier (0-255)
 * - FAN pin: Digital HIGH/LOW controls polarity via DPDT:
 *   • HIGH (S255) → DPDT energized → Peltier HEATING mode
 *   • LOW  (S0)   → DPDT relaxed   → Peltier COOLING mode
 *
 * Usage:
 *   M104 T0 S37  ; Set E0 Peltier target to 37°C (heating)
 *   M104 T1 S4   ; Set E1 Peltier target to 4°C (cooling)
 *   M140 S37     ; Set Bed Peltier target to 37°C
 */

#pragma once

#include "../inc/MarlinConfig.h"

// Common Peltier mode enum
enum PeltierMode : uint8_t {
  PELTIER_OFF,      // Both heater and DPDT off
  PELTIER_HEATING,  // DPDT HIGH (energized), heater PWM
  PELTIER_COOLING   // DPDT LOW (relaxed), heater PWM
};

//===========================================================================
// PELTIER E0 (Extruder 0) - P60/HE0
//===========================================================================
#if ENABLED(PELTIER_CONTROL_E0)

class PeltierControlE0 {
public:
  static PeltierMode current_mode;
  static uint8_t power_pwm;

  static void init();
  static void set_mode(PeltierMode mode, uint8_t pwm = 0);
  static void emergency_stop();
  static PeltierMode get_mode() { return current_mode; }
  static uint8_t get_pwm() { return power_pwm; }

private:
  static void apply_to_hardware();
  static void safe_polarity_change(bool heating_mode);
};

extern PeltierControlE0 peltier_e0;

// Backward compatibility alias
#define PeltierControl PeltierControlE0
#define peltier peltier_e0

#endif // PELTIER_CONTROL_E0

//===========================================================================
// PELTIER E1 (Extruder 1) - P61/HE1
//===========================================================================
#if ENABLED(PELTIER_CONTROL_E1)

class PeltierControlE1 {
public:
  static PeltierMode current_mode;
  static uint8_t power_pwm;

  static void init();
  static void set_mode(PeltierMode mode, uint8_t pwm = 0);
  static void emergency_stop();
  static PeltierMode get_mode() { return current_mode; }
  static uint8_t get_pwm() { return power_pwm; }

private:
  static void apply_to_hardware();
  static void safe_polarity_change(bool heating_mode);
};

extern PeltierControlE1 peltier_e1;

#endif // PELTIER_CONTROL_E1

//===========================================================================
// PELTIER BED - P62/HE2
//===========================================================================
#if ENABLED(PELTIER_CONTROL_BED)

class PeltierControlBed {
public:
  static PeltierMode current_mode;
  static uint8_t power_pwm;

  static void init();
  static void set_mode(PeltierMode mode, uint8_t pwm = 0);
  static void emergency_stop();
  static PeltierMode get_mode() { return current_mode; }
  static uint8_t get_pwm() { return power_pwm; }

private:
  static void apply_to_hardware();
  static void safe_polarity_change(bool heating_mode);
};

extern PeltierControlBed peltier_bed;

#endif // PELTIER_CONTROL_BED
