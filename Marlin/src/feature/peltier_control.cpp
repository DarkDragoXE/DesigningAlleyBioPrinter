/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2025 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Peltier Bidirectional Temperature Control Implementation
 */

#include "../inc/MarlinConfigPre.h"

#if ENABLED(PELTIER_CONTROL_E0)

#include "peltier_control.h"
#include "../module/temperature.h"
#include "../MarlinCore.h"

// Static member initialization
PeltierControl::Mode PeltierControl::current_mode = PELTIER_OFF;
uint8_t PeltierControl::power_pwm = 0;

void PeltierControl::init() {
  // Initialize PA2 (HE0/HEATER_0_PIN) - PWM power control
  #if PIN_EXISTS(HEATER_0)
    OUT_WRITE(HEATER_0_PIN, LOW);  // Start with power OFF
  #else
    #error "HEATER_0_PIN not defined! Check pins configuration."
  #endif

  // Initialize PD12 (FAN2_PIN) - DPDT polarity control via ULN2003
  #if PIN_EXISTS(FAN2)
    OUT_WRITE(FAN2_PIN, LOW);  // Start in COOLING mode (DPDT relaxed)
  #else
    #error "FAN2_PIN not defined! Check pins configuration."
  #endif

  current_mode = PELTIER_OFF;
  power_pwm = 0;

  // Note: No serial output here - serial not initialized yet at this point
}

void PeltierControl::set_mode(Mode mode, uint8_t pwm) {
  // Validate PWM value
  if (pwm > 255) pwm = 255;

  // If mode changed, need safe polarity switching
  if (mode != current_mode) {
    SERIAL_ECHOPGM("Peltier: Mode change ");
    SERIAL_ECHO(current_mode);
    SERIAL_ECHOPGM(" → ");
    SERIAL_ECHOLN(mode);

    // Safety: Turn off power before changing polarity
    if (current_mode != PELTIER_OFF) {
      #if PIN_EXISTS(HEATER_0)
        analogWrite(HEATER_0_PIN, 0);  // PWM = 0
      #endif
      safe_delay(PELTIER_INTERLOCK_DELAY_MS);  // Wait for MOSFET to turn off
    }
  }

  current_mode = mode;
  power_pwm = pwm;

  apply_to_hardware();
}

void PeltierControl::apply_to_hardware() {
  switch (current_mode) {
    case PELTIER_HEATING:
      // Heating mode: PD12 HIGH (energize DPDT), PA2 = PWM
      #if PIN_EXISTS(FAN2)
        WRITE(FAN2_PIN, HIGH);  // DPDT energized → heating polarity
      #endif
      safe_delay(PELTIER_INTERLOCK_DELAY_MS);  // Wait for relay to switch

      #if PIN_EXISTS(HEATER_0)
        analogWrite(HEATER_0_PIN, power_pwm);  // Apply PWM power
      #endif

      #if ENABLED(DEBUG_PELTIER_CONTROL)
        SERIAL_ECHOPGM("Peltier HEATING: PWM=");
        SERIAL_ECHOLN(power_pwm);
      #endif
      break;

    case PELTIER_COOLING:
      // Cooling mode: PD12 LOW (relax DPDT), PA2 = PWM
      #if PIN_EXISTS(FAN2)
        WRITE(FAN2_PIN, LOW);  // DPDT relaxed → cooling polarity
      #endif
      safe_delay(PELTIER_INTERLOCK_DELAY_MS);  // Wait for relay to switch

      #if PIN_EXISTS(HEATER_0)
        analogWrite(HEATER_0_PIN, power_pwm);  // Apply PWM power
      #endif

      #if ENABLED(DEBUG_PELTIER_CONTROL)
        SERIAL_ECHOPGM("Peltier COOLING: PWM=");
        SERIAL_ECHOLN(power_pwm);
      #endif
      break;

    case PELTIER_OFF:
    default:
      // Off mode: Both pins LOW
      #if PIN_EXISTS(HEATER_0)
        analogWrite(HEATER_0_PIN, 0);  // PWM = 0
      #endif
      #if PIN_EXISTS(FAN2)
        WRITE(FAN2_PIN, LOW);  // DPDT relaxed
      #endif

      #if ENABLED(DEBUG_PELTIER_CONTROL)
        SERIAL_ECHOLNPGM("Peltier OFF");
      #endif
      break;
  }
}

void PeltierControl::safe_polarity_change(bool heating_mode) {
  // Ensure MOSFET is fully OFF before changing DPDT
  #if PIN_EXISTS(HEATER_0)
    analogWrite(HEATER_0_PIN, 0);
  #endif

  safe_delay(PELTIER_INTERLOCK_DELAY_MS);

  // Change DPDT polarity
  #if PIN_EXISTS(FAN2)
    WRITE(FAN2_PIN, heating_mode ? HIGH : LOW);
  #endif

  safe_delay(PELTIER_INTERLOCK_DELAY_MS);  // Wait for relay settling
}

void PeltierControl::emergency_stop() {
  SERIAL_ECHOLNPGM("Peltier: EMERGENCY STOP");

  // Immediately turn off power
  #if PIN_EXISTS(HEATER_0)
    analogWrite(HEATER_0_PIN, 0);
  #endif

  // Set to cooling mode (safer default)
  #if PIN_EXISTS(FAN2)
    WRITE(FAN2_PIN, LOW);
  #endif

  current_mode = PELTIER_OFF;
  power_pwm = 0;
}

// Global instance
PeltierControl peltier;

#endif // PELTIER_CONTROL_E0
