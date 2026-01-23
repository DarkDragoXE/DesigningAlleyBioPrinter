/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2025 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Peltier Bidirectional Temperature Control Implementation
 * Supports 3 Peltier modules: E0, E1, and Bed
 */

#include "../inc/MarlinConfigPre.h"

#if ANY(PELTIER_CONTROL_E0, PELTIER_CONTROL_E1, PELTIER_CONTROL_BED)

#include "peltier_control.h"
#include "../module/temperature.h"
#include "../MarlinCore.h"

//===========================================================================
// PELTIER E0 (Extruder 0) - P60 (PD12/FAN2) + HE0 (PA2)
//===========================================================================
#if ENABLED(PELTIER_CONTROL_E0)

// Static member initialization
PeltierMode PeltierControlE0::current_mode = PELTIER_OFF;
uint8_t PeltierControlE0::power_pwm = 0;

void PeltierControlE0::init() {
  // Initialize PA2 (HE0/HEATER_0_PIN) - PWM power control
  #if PIN_EXISTS(HEATER_0)
    OUT_WRITE(HEATER_0_PIN, LOW);  // Start with power OFF
  #else
    #error "HEATER_0_PIN not defined! Check pins configuration."
  #endif

  // Initialize PD12 (FAN2_PIN/P60) - DPDT polarity control via ULN2003
  #if PIN_EXISTS(FAN2)
    OUT_WRITE(FAN2_PIN, LOW);  // Start in COOLING mode (DPDT relaxed)
  #else
    #error "FAN2_PIN not defined! Check pins configuration."
  #endif

  current_mode = PELTIER_OFF;
  power_pwm = 0;
}

void PeltierControlE0::set_mode(PeltierMode mode, uint8_t pwm) {
  if (pwm > 255) pwm = 255;

  if (mode != current_mode) {
    SERIAL_ECHOPGM("Peltier E0: Mode change ");
    SERIAL_ECHO(current_mode);
    SERIAL_ECHOPGM(" -> ");
    SERIAL_ECHOLN(mode);

    if (current_mode != PELTIER_OFF) {
      #if PIN_EXISTS(HEATER_0)
        analogWrite(HEATER_0_PIN, 0);
      #endif
      safe_delay(PELTIER_INTERLOCK_DELAY_MS);
    }
  }

  current_mode = mode;
  power_pwm = pwm;
  apply_to_hardware();
}

void PeltierControlE0::apply_to_hardware() {
  switch (current_mode) {
    case PELTIER_HEATING:
      #if PIN_EXISTS(FAN2)
        WRITE(FAN2_PIN, HIGH);  // P60 HIGH = HEATING
      #endif
      safe_delay(PELTIER_INTERLOCK_DELAY_MS);
      #if PIN_EXISTS(HEATER_0)
        analogWrite(HEATER_0_PIN, power_pwm);
      #endif
      #if ENABLED(DEBUG_PELTIER_CONTROL)
        SERIAL_ECHOPGM("Peltier E0 HEATING: PWM=");
        SERIAL_ECHOLN(power_pwm);
      #endif
      break;

    case PELTIER_COOLING:
      #if PIN_EXISTS(FAN2)
        WRITE(FAN2_PIN, LOW);  // P60 LOW = COOLING
      #endif
      safe_delay(PELTIER_INTERLOCK_DELAY_MS);
      #if PIN_EXISTS(HEATER_0)
        analogWrite(HEATER_0_PIN, power_pwm);
      #endif
      #if ENABLED(DEBUG_PELTIER_CONTROL)
        SERIAL_ECHOPGM("Peltier E0 COOLING: PWM=");
        SERIAL_ECHOLN(power_pwm);
      #endif
      break;

    case PELTIER_OFF:
    default:
      #if PIN_EXISTS(HEATER_0)
        analogWrite(HEATER_0_PIN, 0);
      #endif
      #if PIN_EXISTS(FAN2)
        WRITE(FAN2_PIN, LOW);
      #endif
      #if ENABLED(DEBUG_PELTIER_CONTROL)
        SERIAL_ECHOLNPGM("Peltier E0 OFF");
      #endif
      break;
  }
}

void PeltierControlE0::safe_polarity_change(bool heating_mode) {
  #if PIN_EXISTS(HEATER_0)
    analogWrite(HEATER_0_PIN, 0);
  #endif
  safe_delay(PELTIER_INTERLOCK_DELAY_MS);
  #if PIN_EXISTS(FAN2)
    WRITE(FAN2_PIN, heating_mode ? HIGH : LOW);
  #endif
  safe_delay(PELTIER_INTERLOCK_DELAY_MS);
}

void PeltierControlE0::emergency_stop() {
  SERIAL_ECHOLNPGM("Peltier E0: EMERGENCY STOP");
  #if PIN_EXISTS(HEATER_0)
    analogWrite(HEATER_0_PIN, 0);
  #endif
  #if PIN_EXISTS(FAN2)
    WRITE(FAN2_PIN, LOW);
  #endif
  current_mode = PELTIER_OFF;
  power_pwm = 0;
}

PeltierControlE0 peltier_e0;

#endif // PELTIER_CONTROL_E0

//===========================================================================
// PELTIER E1 (Extruder 1) - P61 (PD13/FAN3) + HE1 (PA3)
//===========================================================================
#if ENABLED(PELTIER_CONTROL_E1)

// Static member initialization
PeltierMode PeltierControlE1::current_mode = PELTIER_OFF;
uint8_t PeltierControlE1::power_pwm = 0;

void PeltierControlE1::init() {
  // Initialize PA3 (HE1/HEATER_1_PIN) - PWM power control
  #if PIN_EXISTS(HEATER_1)
    OUT_WRITE(HEATER_1_PIN, LOW);  // Start with power OFF
  #else
    #error "HEATER_1_PIN not defined! Check pins configuration."
  #endif

  // Initialize PD13 (FAN3_PIN/P61) - DPDT polarity control via ULN2003
  #if PIN_EXISTS(FAN3)
    OUT_WRITE(FAN3_PIN, LOW);  // Start in COOLING mode (DPDT relaxed)
  #else
    #error "FAN3_PIN not defined! Check pins configuration."
  #endif

  current_mode = PELTIER_OFF;
  power_pwm = 0;
}

void PeltierControlE1::set_mode(PeltierMode mode, uint8_t pwm) {
  if (pwm > 255) pwm = 255;

  if (mode != current_mode) {
    SERIAL_ECHOPGM("Peltier E1: Mode change ");
    SERIAL_ECHO(current_mode);
    SERIAL_ECHOPGM(" -> ");
    SERIAL_ECHOLN(mode);

    if (current_mode != PELTIER_OFF) {
      #if PIN_EXISTS(HEATER_1)
        analogWrite(HEATER_1_PIN, 0);
      #endif
      safe_delay(PELTIER_INTERLOCK_DELAY_MS);
    }
  }

  current_mode = mode;
  power_pwm = pwm;
  apply_to_hardware();
}

void PeltierControlE1::apply_to_hardware() {
  switch (current_mode) {
    case PELTIER_HEATING:
      #if PIN_EXISTS(FAN3)
        WRITE(FAN3_PIN, HIGH);  // P61 HIGH = HEATING
      #endif
      safe_delay(PELTIER_INTERLOCK_DELAY_MS);
      #if PIN_EXISTS(HEATER_1)
        analogWrite(HEATER_1_PIN, power_pwm);
      #endif
      #if ENABLED(DEBUG_PELTIER_CONTROL)
        SERIAL_ECHOPGM("Peltier E1 HEATING: PWM=");
        SERIAL_ECHOLN(power_pwm);
      #endif
      break;

    case PELTIER_COOLING:
      #if PIN_EXISTS(FAN3)
        WRITE(FAN3_PIN, LOW);  // P61 LOW = COOLING
      #endif
      safe_delay(PELTIER_INTERLOCK_DELAY_MS);
      #if PIN_EXISTS(HEATER_1)
        analogWrite(HEATER_1_PIN, power_pwm);
      #endif
      #if ENABLED(DEBUG_PELTIER_CONTROL)
        SERIAL_ECHOPGM("Peltier E1 COOLING: PWM=");
        SERIAL_ECHOLN(power_pwm);
      #endif
      break;

    case PELTIER_OFF:
    default:
      #if PIN_EXISTS(HEATER_1)
        analogWrite(HEATER_1_PIN, 0);
      #endif
      #if PIN_EXISTS(FAN3)
        WRITE(FAN3_PIN, LOW);
      #endif
      #if ENABLED(DEBUG_PELTIER_CONTROL)
        SERIAL_ECHOLNPGM("Peltier E1 OFF");
      #endif
      break;
  }
}

void PeltierControlE1::safe_polarity_change(bool heating_mode) {
  #if PIN_EXISTS(HEATER_1)
    analogWrite(HEATER_1_PIN, 0);
  #endif
  safe_delay(PELTIER_INTERLOCK_DELAY_MS);
  #if PIN_EXISTS(FAN3)
    WRITE(FAN3_PIN, heating_mode ? HIGH : LOW);
  #endif
  safe_delay(PELTIER_INTERLOCK_DELAY_MS);
}

void PeltierControlE1::emergency_stop() {
  SERIAL_ECHOLNPGM("Peltier E1: EMERGENCY STOP");
  #if PIN_EXISTS(HEATER_1)
    analogWrite(HEATER_1_PIN, 0);
  #endif
  #if PIN_EXISTS(FAN3)
    WRITE(FAN3_PIN, LOW);
  #endif
  current_mode = PELTIER_OFF;
  power_pwm = 0;
}

PeltierControlE1 peltier_e1;

#endif // PELTIER_CONTROL_E1

//===========================================================================
// PELTIER BED - P62 (PD14/FAN4) + HE2 (PB10)
//===========================================================================
#if ENABLED(PELTIER_CONTROL_BED)

// Static member initialization
PeltierMode PeltierControlBed::current_mode = PELTIER_OFF;
uint8_t PeltierControlBed::power_pwm = 0;

void PeltierControlBed::init() {
  // Initialize PB10 (HE2/HEATER_2_PIN) - PWM power control
  #if PIN_EXISTS(HEATER_2)
    OUT_WRITE(HEATER_2_PIN, LOW);  // Start with power OFF
  #else
    #error "HEATER_2_PIN not defined! Check pins configuration."
  #endif

  // Initialize PD14 (FAN4_PIN/P62) - DPDT polarity control via ULN2003
  #if PIN_EXISTS(FAN4)
    OUT_WRITE(FAN4_PIN, LOW);  // Start in COOLING mode (DPDT relaxed)
  #else
    #error "FAN4_PIN not defined! Check pins configuration."
  #endif

  current_mode = PELTIER_OFF;
  power_pwm = 0;
}

void PeltierControlBed::set_mode(PeltierMode mode, uint8_t pwm) {
  if (pwm > 255) pwm = 255;

  if (mode != current_mode) {
    SERIAL_ECHOPGM("Peltier Bed: Mode change ");
    SERIAL_ECHO(current_mode);
    SERIAL_ECHOPGM(" -> ");
    SERIAL_ECHOLN(mode);

    if (current_mode != PELTIER_OFF) {
      #if PIN_EXISTS(HEATER_2)
        analogWrite(HEATER_2_PIN, 0);
      #endif
      safe_delay(PELTIER_INTERLOCK_DELAY_MS);
    }
  }

  current_mode = mode;
  power_pwm = pwm;
  apply_to_hardware();
}

void PeltierControlBed::apply_to_hardware() {
  switch (current_mode) {
    case PELTIER_HEATING:
      #if PIN_EXISTS(FAN4)
        WRITE(FAN4_PIN, HIGH);  // P62 HIGH = HEATING
      #endif
      safe_delay(PELTIER_INTERLOCK_DELAY_MS);
      #if PIN_EXISTS(HEATER_2)
        analogWrite(HEATER_2_PIN, power_pwm);
      #endif
      #if ENABLED(DEBUG_PELTIER_CONTROL)
        SERIAL_ECHOPGM("Peltier Bed HEATING: PWM=");
        SERIAL_ECHOLN(power_pwm);
      #endif
      break;

    case PELTIER_COOLING:
      #if PIN_EXISTS(FAN4)
        WRITE(FAN4_PIN, LOW);  // P62 LOW = COOLING
      #endif
      safe_delay(PELTIER_INTERLOCK_DELAY_MS);
      #if PIN_EXISTS(HEATER_2)
        analogWrite(HEATER_2_PIN, power_pwm);
      #endif
      #if ENABLED(DEBUG_PELTIER_CONTROL)
        SERIAL_ECHOPGM("Peltier Bed COOLING: PWM=");
        SERIAL_ECHOLN(power_pwm);
      #endif
      break;

    case PELTIER_OFF:
    default:
      #if PIN_EXISTS(HEATER_2)
        analogWrite(HEATER_2_PIN, 0);
      #endif
      #if PIN_EXISTS(FAN4)
        WRITE(FAN4_PIN, LOW);
      #endif
      #if ENABLED(DEBUG_PELTIER_CONTROL)
        SERIAL_ECHOLNPGM("Peltier Bed OFF");
      #endif
      break;
  }
}

void PeltierControlBed::safe_polarity_change(bool heating_mode) {
  #if PIN_EXISTS(HEATER_2)
    analogWrite(HEATER_2_PIN, 0);
  #endif
  safe_delay(PELTIER_INTERLOCK_DELAY_MS);
  #if PIN_EXISTS(FAN4)
    WRITE(FAN4_PIN, heating_mode ? HIGH : LOW);
  #endif
  safe_delay(PELTIER_INTERLOCK_DELAY_MS);
}

void PeltierControlBed::emergency_stop() {
  SERIAL_ECHOLNPGM("Peltier Bed: EMERGENCY STOP");
  #if PIN_EXISTS(HEATER_2)
    analogWrite(HEATER_2_PIN, 0);
  #endif
  #if PIN_EXISTS(FAN4)
    WRITE(FAN4_PIN, LOW);
  #endif
  current_mode = PELTIER_OFF;
  power_pwm = 0;
}

PeltierControlBed peltier_bed;

#endif // PELTIER_CONTROL_BED

#endif // ANY(PELTIER_CONTROL_E0, PELTIER_CONTROL_E1, PELTIER_CONTROL_BED)
