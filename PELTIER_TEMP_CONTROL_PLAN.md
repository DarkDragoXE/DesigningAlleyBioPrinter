# Peltier Temperature Control Implementation Plan
## Bioprinter E0 Extruder with Heating/Cooling Capability

**Objective:** Implement bidirectional temperature control (heating & cooling) for E0 extruder using a Peltier element controlled via DPDT relay + ULN2003 driver system.

---

## Table of Contents

1. [System Overview](#1-system-overview)
2. [Hardware Requirements](#2-hardware-requirements)
3. [Pin Assignment Strategy](#3-pin-assignment-strategy)
4. [Software Architecture](#4-software-architecture)
5. [Detailed Implementation Plan](#5-detailed-implementation-plan)
6. [Testing & Validation](#6-testing--validation)
7. [Safety Considerations](#7-safety-considerations)

---

## 1. System Overview

### 1.1 Hardware Configuration

```
┌─────────────┐
│   Marlin    │
│  Firmware   │
└──────┬──────┘
       │ GPIO Signals
       ↓
┌──────────────────────────┐
│  BTT Octopus Control     │
│  - HEAT_PIN (PWM)        │ ← Heating power control
│  - COOL_PIN (PWM)        │ ← Cooling power control
│  - POLARITY_PIN (Digital)│ ← DPDT relay control
└──────┬───────────────────┘
       │
       ↓
┌──────────────────────────┐
│  ULN2003 Driver Array    │
│  - Channel 1: Heat relay │
│  - Channel 2: Cool relay │
│  - Channel 3: DPDT relay │
└──────┬───────────────────┘
       │
       ↓
┌──────────────────────────┐
│  DPDT Relay             │
│  (Polarity Switching)    │
└──────┬───────────────────┘
       │
       ↓
┌──────────────────────────┐
│  Peltier Element         │
│  (TEC1-12706 or similar) │
└──────┬───────────────────┘
       │
       ↓
┌──────────────────────────┐
│  Thermistor (NTC 100K)   │
│  Temperature Feedback    │
└──────────────────────────┘
```

### 1.2 Control Logic

**Temperature Range:** 4°C to 40°C (capped)

**Operating Modes:**
- **HEATING MODE:** Target temp > Current temp → Peltier hot side to extruder
- **COOLING MODE:** Target temp < Current temp → Peltier cold side to extruder
- **IDLE MODE:** Target temp ≈ Current temp (within deadband) → Peltier OFF

**Control Method:** PID with mode switching

---

## 2. Hardware Requirements

### 2.1 Peltier Module Specifications
- **Type:** TEC1-12706 (12V, 6A max) or equivalent
- **Voltage:** 12V DC
- **Current:** Up to 6A (72W max)
- **Temperature Range:** -40°C to +80°C differential

### 2.2 DPDT Relay
- **Type:** 5V coil, 10A contacts minimum
- **Purpose:** Reverse polarity to switch between heating/cooling
- **Example:** Omron G5V-2 or equivalent

### 2.3 ULN2003 Driver
- **Channels Used:** 3 (Heat control, Cool control, DPDT polarity)
- **Input:** 5V TTL from Octopus GPIO
- **Output:** Sinks current to drive relay coils

### 2.4 Thermistor
- **Type:** NTC 100K (Beta 3950)
- **Connection:** Existing E0 thermistor input (T0 on Octopus)
- **Temperature Range:** 0°C to 50°C (suitable for bioprinting)

### 2.5 Power Supply
- **Peltier Power:** 12V DC, 8A minimum (with margin)
- **Logic Power:** 5V from Octopus board
- **Relay Power:** 5V or 12V depending on relay coil

---

## 3. Pin Assignment Strategy

### 3.1 BTT Octopus V1.1 Available Pins

**Option 1: Use Heater and Fan Pins (Recommended)**
- **HEAT_PIN:** `PE5` (HEATER_BED pin) - PWM capable
- **COOL_PIN:** `PD13` (FAN0 pin) - PWM capable
- **POLARITY_PIN:** `PD14` (FAN1 pin) - Digital output

**Option 2: Use Unused Motor Pins**
- **HEAT_PIN:** `PF11` (MOTOR_5 STEP)
- **COOL_PIN:** `PC13` (MOTOR_5 DIR)
- **POLARITY_PIN:** `PF13` (MOTOR_5 ENABLE)

**Option 3: Use Servo/Probe Pins**
- **HEAT_PIN:** `PE5` (HEATER_BED)
- **COOL_PIN:** `PB11` (SERVO0)
- **POLARITY_PIN:** `PG8` (PROBE pin, if not used)

### 3.2 Recommended Configuration

**Using Bed Heater + Fan Pins:**
```cpp
// Pin Definitions for Peltier Control
#define E0_PELTIER_HEAT_PIN   PE5   // Bed heater output (PWM)
#define E0_PELTIER_COOL_PIN   PD13  // FAN0 output (PWM)
#define E0_PELTIER_POLARITY_PIN PD14 // FAN1 output (Digital)
```

**Rationale:**
- PE5 (bed heater) has hardware PWM and high current capability
- PD13 and PD14 (fan outputs) are designed for inductive loads
- All pins are 5V tolerant and can drive ULN2003 directly
- Does not interfere with E0 motor or E1 extruder

### 3.3 Thermistor Connection
- **Pin:** PA3 (T0 - Extruder 0 thermistor input)
- **Already configured** for E0 temperature sensing
- **Thermistor Type:** Type 1 (100K thermistor with 4.7K pullup)

---

## 4. Software Architecture

### 4.1 Temperature Control Framework

Marlin already has a robust temperature control system. We need to:

1. **Extend PID logic** to support bidirectional control
2. **Add polarity switching** based on heating/cooling mode
3. **Implement mode detection** (heat vs cool vs idle)
4. **Add safety limits** (4°C to 40°C cap)
5. **Support standard G-code** (M104, M109 for E0)

### 4.2 Control Algorithm

```
┌─────────────────────────────────┐
│  Temperature Manager ISR        │
│  (Called every ~250ms)          │
└────────────┬────────────────────┘
             │
             ↓
      ┌──────────────┐
      │ Read Temp    │
      │ (Thermistor) │
      └──────┬───────┘
             │
             ↓
      ┌──────────────────┐
      │ Check Safety     │
      │ (4°C - 40°C cap) │
      └──────┬───────────┘
             │
             ↓
   ┌─────────────────────┐
   │ Calculate Error     │
   │ error = target-curr │
   └─────────┬───────────┘
             │
             ↓
   ┌─────────────────────────┐
   │ Determine Mode          │
   │ if error > +deadband    │ → HEATING MODE
   │ if error < -deadband    │ → COOLING MODE
   │ if |error| ≤ deadband   │ → IDLE MODE
   └─────────┬───────────────┘
             │
      ┌──────┴──────────┐
      ↓                 ↓
┌──────────────┐  ┌──────────────┐
│ HEATING MODE │  │ COOLING MODE │
└──────┬───────┘  └──────┬───────┘
       │                 │
       ↓                 ↓
┌─────────────────┐ ┌─────────────────┐
│ Set POLARITY=0  │ │ Set POLARITY=1  │
│ (Hot→Extruder)  │ │ (Cold→Extruder) │
└────────┬────────┘ └────────┬────────┘
         │                   │
         ↓                   ↓
   ┌─────────────────────────┐
   │ Run PID Controller      │
   │ output = Kp*e + Ki*∫e   │
   │          + Kd*de/dt     │
   └─────────┬───────────────┘
             │
             ↓
   ┌─────────────────────┐
   │ Apply PWM to Pins   │
   │ HEAT_PIN or COOL_PIN│
   └─────────────────────┘
```

### 4.3 PID Parameters

**Initial Tuning Values (to be calibrated):**
```cpp
#define DEFAULT_Kp_E0_PELTIER  20.0   // Proportional gain
#define DEFAULT_Ki_E0_PELTIER  1.5    // Integral gain
#define DEFAULT_Kd_E0_PELTIER  50.0   // Derivative gain
#define E0_PELTIER_DEADBAND    0.5    // °C deadband for mode switching
```

**Why Different PID Values:**
- Peltier has different thermal response than resistive heater
- Bidirectional control requires different tuning
- Cooling is typically slower than heating

---

## 5. Detailed Implementation Plan

### 5.1 Phase 1: Configuration Files

#### File: `Configuration.h`

**Location: Line ~600 (Extruder Temperature Settings)**

```cpp
//===========================================================================
//============================= Peltier Settings ============================
//===========================================================================

/**
 * Peltier Temperature Control for E0 Extruder
 *
 * Enables bidirectional temperature control (heating and cooling) using
 * a Peltier element with DPDT relay for polarity switching.
 *
 * Hardware Requirements:
 * - Peltier element (TEC1-12706 or equivalent)
 * - DPDT relay (5V/12V coil, 10A contacts)
 * - ULN2003 driver IC
 * - NTC 100K thermistor
 */
#define E0_PELTIER_CONTROL

#if ENABLED(E0_PELTIER_CONTROL)
  // Temperature limits for bioprinting applications
  #define E0_PELTIER_MINTEMP   4     // Minimum allowed temperature (°C)
  #define E0_PELTIER_MAXTEMP   40    // Maximum allowed temperature (°C)

  // PID tuning values (adjust after auto-tuning)
  #define DEFAULT_Kp_E0_PELTIER  20.0
  #define DEFAULT_Ki_E0_PELTIER  1.5
  #define DEFAULT_Kd_E0_PELTIER  50.0

  // Mode switching deadband (prevents oscillation)
  #define E0_PELTIER_DEADBAND  0.5   // °C

  // Safety timeout (disable if temp not reached in X seconds)
  #define E0_PELTIER_TIMEOUT  (5*60) // 5 minutes
#endif
```

**Location: Line ~470 (Thermistor Settings)**

```cpp
// Verify E0 thermistor is configured
#define TEMP_SENSOR_0 1  // 100k thermistor - VERIFY THIS IS SET
```

#### File: `Configuration_adv.h`

**Location: Line ~343 (Advanced PID Settings)**

```cpp
#if ENABLED(E0_PELTIER_CONTROL)
  /**
   * Peltier PID Auto-Tune
   *
   * Use M303 E0 S30 C8 to auto-tune PID parameters
   * Target temperature: 30°C (mid-range for bioprinting)
   * Cycles: 8 (sufficient for convergence)
   */
  #define PELTIER_PID_AUTOTUNE

  /**
   * Functional Range
   *
   * Define useful temperature range for bioprinting:
   * - Cell cultures: 4-10°C (cooling)
   * - Hydrogels: 15-25°C (moderate)
   * - Thermoplastics: 30-40°C (heating)
   */
  #define PELTIER_FUNCTIONAL_RANGE { 4, 40 }

  /**
   * Power Limits
   *
   * Limit maximum PWM duty cycle to prevent:
   * - Excessive current draw
   * - Thermal runaway
   * - Component damage
   */
  #define PELTIER_MAX_POWER_HEAT  200  // Max PWM value for heating (0-255)
  #define PELTIER_MAX_POWER_COOL  200  // Max PWM value for cooling (0-255)
#endif
```

### 5.2 Phase 2: Pin Definitions

#### File: `pins_BTT_OCTOPUS_V1_common.h`

**Location: After line ~240 (around extruder pins)**

```cpp
//
// Peltier Temperature Control for E0
//
#if ENABLED(E0_PELTIER_CONTROL)
  #ifndef E0_PELTIER_HEAT_PIN
    #define E0_PELTIER_HEAT_PIN     PE5   // Bed heater pin (PWM capable)
  #endif
  #ifndef E0_PELTIER_COOL_PIN
    #define E0_PELTIER_COOL_PIN     PD13  // FAN0 pin (PWM capable)
  #endif
  #ifndef E0_PELTIER_POLARITY_PIN
    #define E0_PELTIER_POLARITY_PIN PD14  // FAN1 pin (Digital output)
  #endif

  // Disable conflicting outputs
  #undef HEATER_BED_PIN  // Using PE5 for Peltier heat control
  #undef FAN_PIN         // Using PD13 for Peltier cool control
  #undef FAN1_PIN        // Using PD14 for DPDT relay control
#endif
```

### 5.3 Phase 3: Temperature Manager Modifications

#### File: `temperature.h` (Marlin/src/module/temperature.h)

**Location: Class definition (~line 450)**

```cpp
class Temperature {
  public:
    // Existing members...

    #if ENABLED(E0_PELTIER_CONTROL)
      // Peltier control state
      enum PeltierMode : uint8_t {
        PELTIER_OFF,      // Not active
        PELTIER_HEATING,  // Hot side to extruder
        PELTIER_COOLING,  // Cold side to extruder
        PELTIER_IDLE      // At target, maintaining
      };

      static PeltierMode peltier_mode;
      static float peltier_target;
      static float peltier_current;

      // PID state for Peltier
      static float peltier_pid_output;
      static float peltier_pid_integral;
      static float peltier_pid_last_error;

      // Control methods
      static void manage_peltier();
      static void set_peltier_target(const float &celsius);
      static float get_peltier_temp() { return peltier_current; }
      static void peltier_set_polarity(const bool heating);
      static void peltier_apply_power(const int16_t power);

      // Safety
      static void check_peltier_temp();
      static bool peltier_temp_error;
      static millis_t peltier_timeout_start;

      // Auto-tune
      #if ENABLED(PELTIER_PID_AUTOTUNE)
        static void peltier_PID_autotune(const float target, const int ncycles);
      #endif
    #endif

  private:
    // Existing members...
};
```

#### File: `temperature.cpp` (Marlin/src/module/temperature.cpp)

**Location: Add after existing temperature management code**

```cpp
#if ENABLED(E0_PELTIER_CONTROL)

// Static member initialization
Temperature::PeltierMode Temperature::peltier_mode = PELTIER_OFF;
float Temperature::peltier_target = 0.0f;
float Temperature::peltier_current = 0.0f;
float Temperature::peltier_pid_output = 0.0f;
float Temperature::peltier_pid_integral = 0.0f;
float Temperature::peltier_pid_last_error = 0.0f;
bool Temperature::peltier_temp_error = false;
millis_t Temperature::peltier_timeout_start = 0;

/**
 * Set Peltier Target Temperature
 *
 * Clamps temperature to safe range and initializes timeout
 */
void Temperature::set_peltier_target(const float &celsius) {
  // Clamp to safe limits
  peltier_target = constrain(celsius, E0_PELTIER_MINTEMP, E0_PELTIER_MAXTEMP);

  // Reset timeout
  peltier_timeout_start = millis();
  peltier_temp_error = false;

  // Reset PID state when target changes significantly
  if (ABS(celsius - peltier_current) > 5.0f) {
    peltier_pid_integral = 0.0f;
    peltier_pid_last_error = 0.0f;
  }

  SERIAL_ECHOLNPGM("Peltier target set to ", celsius, "°C");
}

/**
 * Set DPDT Relay Polarity
 *
 * @param heating  true = hot side to extruder, false = cold side to extruder
 */
void Temperature::peltier_set_polarity(const bool heating) {
  WRITE(E0_PELTIER_POLARITY_PIN, heating ? LOW : HIGH);
  // Wait for relay to switch (10ms typical)
  safe_delay(15);
}

/**
 * Apply Power to Peltier
 *
 * @param power  PWM value (0-255), positive = heat, negative = cool
 */
void Temperature::peltier_apply_power(const int16_t power) {
  if (power > 0) {
    // Heating mode
    const uint8_t heat_pwm = constrain(power, 0, PELTIER_MAX_POWER_HEAT);
    analogWrite(E0_PELTIER_HEAT_PIN, heat_pwm);
    analogWrite(E0_PELTIER_COOL_PIN, 0);
    peltier_set_polarity(true);
    peltier_mode = PELTIER_HEATING;
  }
  else if (power < 0) {
    // Cooling mode
    const uint8_t cool_pwm = constrain(-power, 0, PELTIER_MAX_POWER_COOL);
    analogWrite(E0_PELTIER_HEAT_PIN, 0);
    analogWrite(E0_PELTIER_COOL_PIN, cool_pwm);
    peltier_set_polarity(false);
    peltier_mode = PELTIER_COOLING;
  }
  else {
    // Off
    analogWrite(E0_PELTIER_HEAT_PIN, 0);
    analogWrite(E0_PELTIER_COOL_PIN, 0);
    peltier_mode = PELTIER_IDLE;
  }
}

/**
 * Main Peltier Temperature Management
 *
 * Called from Temperature ISR (~4Hz)
 */
void Temperature::manage_peltier() {
  // Read current temperature
  peltier_current = degHotend(0);  // Use E0 thermistor reading

  // Safety check
  check_peltier_temp();
  if (peltier_temp_error) {
    peltier_apply_power(0);  // Shut down on error
    return;
  }

  // Calculate error
  const float error = peltier_target - peltier_current;

  // Check if target is disabled (0 = off)
  if (peltier_target < E0_PELTIER_MINTEMP) {
    peltier_apply_power(0);
    return;
  }

  // Determine mode based on error and deadband
  if (ABS(error) <= E0_PELTIER_DEADBAND) {
    // Within deadband - minimal power to maintain
    peltier_apply_power(0);
    peltier_mode = PELTIER_IDLE;
    return;
  }

  // Calculate PID output
  const float Kp = DEFAULT_Kp_E0_PELTIER;
  const float Ki = DEFAULT_Ki_E0_PELTIER;
  const float Kd = DEFAULT_Kd_E0_PELTIER;

  // Proportional term
  const float p_term = Kp * error;

  // Integral term (with anti-windup)
  peltier_pid_integral += Ki * error;
  peltier_pid_integral = constrain(peltier_pid_integral, -255, 255);

  // Derivative term
  const float d_term = Kd * (error - peltier_pid_last_error);
  peltier_pid_last_error = error;

  // Calculate total PID output
  int16_t pid_output = (int16_t)(p_term + peltier_pid_integral + d_term);

  // Apply power limits
  pid_output = constrain(pid_output, -PELTIER_MAX_POWER_COOL, PELTIER_MAX_POWER_HEAT);

  // Apply to hardware
  peltier_apply_power(pid_output);

  // Debug output (remove after testing)
  #if ENABLED(TEMP_DEBUG)
    SERIAL_ECHOLNPGM("Peltier: T=", peltier_target, " C=", peltier_current,
                     " E=", error, " PWM=", pid_output, " Mode=", (int)peltier_mode);
  #endif
}

/**
 * Check Peltier Temperature Safety
 */
void Temperature::check_peltier_temp() {
  // Check timeout
  if (peltier_target > E0_PELTIER_MINTEMP) {
    const millis_t elapsed = millis() - peltier_timeout_start;
    if (elapsed > E0_PELTIER_TIMEOUT * 1000UL) {
      const float error = ABS(peltier_target - peltier_current);
      if (error > 2.0f) {  // Not within 2°C after timeout
        peltier_temp_error = true;
        SERIAL_ERROR_MSG("Peltier timeout - target not reached");
        kill(GET_TEXT_F(MSG_THERMAL_RUNAWAY));
      }
    }
  }

  // Check temperature limits
  if (peltier_current < (E0_PELTIER_MINTEMP - 5) ||
      peltier_current > (E0_PELTIER_MAXTEMP + 5)) {
    peltier_temp_error = true;
    SERIAL_ERROR_MSG("Peltier temp out of range: ", peltier_current);
    kill(GET_TEXT_F(MSG_THERMAL_RUNAWAY));
  }
}

/**
 * Initialize Peltier Control
 */
void Temperature::init_peltier() {
  // Set pin modes
  SET_OUTPUT(E0_PELTIER_HEAT_PIN);
  SET_OUTPUT(E0_PELTIER_COOL_PIN);
  SET_OUTPUT(E0_PELTIER_POLARITY_PIN);

  // Initialize to off state
  analogWrite(E0_PELTIER_HEAT_PIN, 0);
  analogWrite(E0_PELTIER_COOL_PIN, 0);
  WRITE(E0_PELTIER_POLARITY_PIN, LOW);

  // Initialize state
  peltier_mode = PELTIER_OFF;
  peltier_target = 0.0f;
  peltier_current = 0.0f;
  peltier_pid_output = 0.0f;
  peltier_pid_integral = 0.0f;
  peltier_pid_last_error = 0.0f;
  peltier_temp_error = false;

  SERIAL_ECHOLNPGM("Peltier control initialized");
}

#endif // E0_PELTIER_CONTROL
```

**Location: In Temperature::init() method**

```cpp
void Temperature::init() {
  // Existing initialization code...

  #if ENABLED(E0_PELTIER_CONTROL)
    init_peltier();
  #endif
}
```

**Location: In Temperature::task() or main ISR**

```cpp
void Temperature::task() {
  // Existing temperature management...

  #if ENABLED(E0_PELTIER_CONTROL)
    manage_peltier();
  #endif
}
```

### 5.4 Phase 4: G-code Integration

#### File: `M104.cpp` (Marlin/src/gcode/temp/M104.cpp)

**Modify to support Peltier control:**

```cpp
void GcodeSuite::M104() {
  const uint8_t e = E_TERN0(get_target_e_stepper_from_command(-2));
  if (e == 0 && ENABLED(E0_PELTIER_CONTROL)) {
    // E0 uses Peltier control
    if (parser.seenval('S')) {
      const float temp = parser.value_celsius();
      thermalManager.set_peltier_target(temp);
    }
  }
  else {
    // Standard hotend control
    // ... existing code ...
  }
}
```

#### File: `M109.cpp` (Marlin/src/gcode/temp/M109.cpp)

**Modify to wait for Peltier temperature:**

```cpp
void GcodeSuite::M109() {
  const uint8_t e = E_TERN0(get_target_e_stepper_from_command(-2));

  #if ENABLED(E0_PELTIER_CONTROL)
    if (e == 0) {
      // E0 uses Peltier - set and wait
      if (parser.seenval('S')) {
        const float temp = parser.value_celsius();
        thermalManager.set_peltier_target(temp);

        // Wait for temperature
        const bool no_wait = parser.seenval('F');
        if (!no_wait) {
          LCD_MESSAGE(MSG_HEATING);
          const millis_t start_time = millis();

          while (ABS(thermalManager.peltier_target - thermalManager.peltier_current) > 0.5f) {
            idle(true);
            gcode.reset_stepper_timeout();

            // Safety timeout
            if (millis() - start_time > E0_PELTIER_TIMEOUT * 1000UL) {
              SERIAL_ERROR_MSG("Peltier timeout");
              break;
            }
          }

          LCD_MESSAGE(MSG_HEATING_COMPLETE);
        }
      }
      return;
    }
  #endif

  // Standard hotend code for other extruders
  // ... existing code ...
}
```

#### File: `M105.cpp` (Marlin/src/gcode/temp/M105.cpp)

**Add Peltier temperature reporting:**

```cpp
void GcodeSuite::M105() {
  // Existing hotend reporting...

  #if ENABLED(E0_PELTIER_CONTROL)
    SERIAL_ECHOPGM(" T0:", thermalManager.peltier_current, "/", thermalManager.peltier_target);
    SERIAL_ECHOPGM(" @0:", thermalManager.peltier_pid_output);
  #endif

  // Existing bed/chamber reporting...
}
```

### 5.5 Phase 5: LCD Menu Integration

#### File: `menu_temperature.cpp` (Marlin/src/lcd/menu/menu_temperature.cpp)

**Location: Temperature menu section**

```cpp
#if ENABLED(E0_PELTIER_CONTROL)

// Peltier temperature presets for bioprinting
void _peltier_temp_preset_4()  { thermalManager.set_peltier_target(4);  }
void _peltier_temp_preset_10() { thermalManager.set_peltier_target(10); }
void _peltier_temp_preset_20() { thermalManager.set_peltier_target(20); }
void _peltier_temp_preset_30() { thermalManager.set_peltier_target(30); }
void _peltier_temp_preset_37() { thermalManager.set_peltier_target(37); }
void _peltier_temp_preset_off() { thermalManager.set_peltier_target(0); }

void menu_peltier_temp() {
  START_MENU();
  BACK_ITEM(MSG_TEMPERATURE);

  // Current temperature display
  STATIC_ITEM_P(PSTR("Current:"), SS_DEFAULT, ftostr3(thermalManager.peltier_current), PSTR("°C"));
  STATIC_ITEM_P(PSTR("Target:"), SS_DEFAULT, ftostr3(thermalManager.peltier_target), PSTR("°C"));

  // Mode indicator
  const char* mode_str;
  switch (thermalManager.peltier_mode) {
    case Temperature::PELTIER_HEATING: mode_str = "HEATING"; break;
    case Temperature::PELTIER_COOLING: mode_str = "COOLING"; break;
    case Temperature::PELTIER_IDLE:    mode_str = "IDLE"; break;
    default:                           mode_str = "OFF"; break;
  }
  STATIC_ITEM_P(PSTR("Mode:"), SS_DEFAULT, mode_str);

  // Temperature presets
  ACTION_ITEM(MSG_PELTIER_OFF,      _peltier_temp_preset_off);
  ACTION_ITEM(MSG_PELTIER_COOL_4C,  _peltier_temp_preset_4);
  ACTION_ITEM(MSG_PELTIER_COOL_10C, _peltier_temp_preset_10);
  ACTION_ITEM(MSG_PELTIER_WARM_20C, _peltier_temp_preset_20);
  ACTION_ITEM(MSG_PELTIER_WARM_30C, _peltier_temp_preset_30);
  ACTION_ITEM(MSG_PELTIER_BODY_37C, _peltier_temp_preset_37);

  // Manual temperature entry
  EDIT_ITEM(int3, MSG_PELTIER_MANUAL, &thermalManager.peltier_target,
            E0_PELTIER_MINTEMP, E0_PELTIER_MAXTEMP);

  END_MENU();
}

#endif // E0_PELTIER_CONTROL
```

**Add to main temperature menu:**

```cpp
void menu_temperature() {
  START_MENU();
  BACK_ITEM(MSG_MAIN);

  #if ENABLED(E0_PELTIER_CONTROL)
    SUBMENU(MSG_PELTIER_TEMP, menu_peltier_temp);
  #endif

  // Existing temperature menu items...

  END_MENU();
}
```

#### File: `language_en.h`

**Add language strings:**

```cpp
#if ENABLED(E0_PELTIER_CONTROL)
  LSTR MSG_PELTIER_TEMP       = _UxGT("Peltier Temp");
  LSTR MSG_PELTIER_OFF        = _UxGT("Off (0°C)");
  LSTR MSG_PELTIER_COOL_4C    = _UxGT("Cool (4°C)");
  LSTR MSG_PELTIER_COOL_10C   = _UxGT("Cool (10°C)");
  LSTR MSG_PELTIER_WARM_20C   = _UxGT("Warm (20°C)");
  LSTR MSG_PELTIER_WARM_30C   = _UxGT("Warm (30°C)");
  LSTR MSG_PELTIER_BODY_37C   = _UxGT("Body (37°C)");
  LSTR MSG_PELTIER_MANUAL     = _UxGT("Manual Temp");
#endif
```

### 5.6 Phase 6: Slicer Integration

**G-code Usage:**

```gcode
; Bioprinting Start G-code (for 4°C material)
M104 S4          ; Set Peltier to 4°C (cooling mode)
M109 S4          ; Wait for 4°C to be reached
G28              ; Home all axes
; ... rest of start code ...

; During print
M104 S20         ; Adjust to 20°C if needed

; End G-code
M104 S0          ; Turn off Peltier
```

**Slicer Configuration Examples:**

**PrusaSlicer / SuperSlicer:**
- Filament Settings > Temperature > First Layer: 4-40°C
- Filament Settings > Temperature > Other Layers: 4-40°C

**Cura:**
- Material Settings > Printing Temperature: 4-40°C
- Material Settings > Build Plate Temperature: (leave at 0 or bed temp)

**Simplify3D:**
- Process Settings > Temperature > Extruder: 4-40°C

---

## 6. Testing & Validation

### 6.1 Hardware Testing

**Step 1: Continuity Tests (Power OFF)**
```
1. Verify ULN2003 connections
2. Verify DPDT relay coil connections
3. Verify Peltier element polarity marking
4. Verify thermistor connections (should read ~100kΩ at 25°C)
```

**Step 2: Polarity Switching Test**
```
1. Flash firmware with Peltier support
2. Send M302 P1 (allow cold extrusion for testing)
3. Send M104 S30 (should enter HEATING mode)
4. Measure voltage at Peltier terminals with multimeter
5. Send M104 S4 (should enter COOLING mode)
6. Verify voltage polarity reversed
```

**Step 3: Temperature Response Test**
```
1. Set target to 30°C (M104 S30)
2. Monitor temperature with M105 every 10 seconds
3. Plot temperature curve (should reach 30°C in <3 minutes)
4. Observe overshoot (should be <2°C)
5. Set target to 10°C (M104 S10)
6. Monitor cooling (should reach 10°C in <5 minutes)
```

### 6.2 PID Auto-Tuning

**For Heating (30°C target):**
```gcode
M303 E0 S30 C8   ; Auto-tune at 30°C for 8 cycles
M500             ; Save results to EEPROM
```

**For Cooling (10°C target):**
```gcode
M303 E0 S10 C8   ; Auto-tune at 10°C for 8 cycles
M500             ; Save results to EEPROM
```

**Expected Results:**
- Kp: 15-30
- Ki: 1-3
- Kd: 40-80

**Manual Tuning If Needed:**
```gcode
M301 P20.0 I1.5 D50.0   ; Set PID values manually
M500                     ; Save to EEPROM
```

### 6.3 Safety Testing

**Test 1: Over-Temperature Protection**
```
1. Set target to 50°C (above max)
2. Verify firmware caps at 40°C
3. Verify no error messages
```

**Test 2: Under-Temperature Protection**
```
1. Set target to 0°C
2. Verify Peltier turns off (not cooling to 0°C)
```

**Test 3: Thermal Runaway**
```
1. Disconnect thermistor (simulate sensor failure)
2. Set target to 20°C
3. Verify firmware detects error and shuts down
4. Verify error message on LCD
```

**Test 4: Timeout Protection**
```
1. Set target to 40°C with Peltier disconnected
2. Wait 5 minutes
3. Verify timeout error triggers
```

### 6.4 Functional Testing

**Test 1: G-code Compatibility**
```gcode
M104 S20         ; Set temp (non-blocking)
M105             ; Report temp (should show T0:XX.X/20.0)
M109 S20         ; Set temp (blocking - wait)
M105             ; Should show temp near 20°C
M104 S0          ; Turn off
```

**Test 2: LCD Menu**
```
1. Navigate to Temperature > Peltier Temp
2. Verify current/target display updates
3. Select "Cool (4°C)" preset
4. Verify temperature drops to 4°C
5. Select "Body (37°C)" preset
6. Verify temperature rises to 37°C
```

**Test 3: Mode Switching**
```
1. Set target to 10°C (cooling)
2. Verify POLARITY_PIN is HIGH
3. Verify COOL_PIN has PWM
4. Set target to 30°C (heating)
5. Verify POLARITY_PIN is LOW
6. Verify HEAT_PIN has PWM
```

---

## 7. Safety Considerations

### 7.1 Electrical Safety

**Power Supply:**
- Use isolated 12V supply for Peltier (min 8A capacity)
- Fuse protection: 10A fast-blow fuse on Peltier circuit
- Proper grounding of all metal components

**Relay Protection:**
- Flyback diodes across relay coils (ULN2003 has internal)
- Snubber circuit across Peltier (0.1µF + 100Ω in series)
- Heat sink for Peltier cold side (forced air cooling recommended)

### 7.2 Thermal Safety

**Firmware Protections:**
1. **MINTEMP / MAXTEMP:** 4°C / 40°C hard limits
2. **Thermal Runaway:** Timeout if temp not reached in 5 minutes
3. **Temperature Sensor Failure:** Shutdown if reading out of range
4. **Deadband:** 0.5°C prevents mode oscillation

**Hardware Protections:**
1. **Thermal Fuse:** 60°C thermal fuse on hot side (backup)
2. **Over-Current Protection:** Circuit breaker or electronic fuse
3. **Condensation Prevention:** Keep cold side >4°C to avoid condensation

### 7.3 Mechanical Safety

**Peltier Mounting:**
- Thermal compound between Peltier and extruder
- Secure mounting to prevent movement
- Insulation around cold side to prevent condensation
- Airflow management to prevent overheating

**Wiring:**
- Strain relief on all connections
- Heat-resistant wiring (silicone jacket rated >150°C)
- Proper gauge wire (14 AWG or larger for Peltier power)

### 7.4 Operational Safety

**User Guidelines:**
1. Never exceed 40°C (material degradation risk)
2. Never go below 4°C (condensation risk)
3. Wait for temperature stabilization before printing
4. Monitor first few prints closely
5. Keep emergency stop accessible

**Maintenance:**
1. Monthly inspection of relay contacts
2. Check thermal compound annually
3. Verify thermistor readings periodically
4. Clean dust from heat sink fins

---

## 8. Troubleshooting Guide

### Problem 1: Temperature Not Changing

**Symptoms:** Target set, but current temp doesn't move

**Checks:**
1. Verify HEAT_PIN and COOL_PIN have voltage (use multimeter or LED)
2. Check ULN2003 input signals with oscilloscope
3. Verify DPDT relay clicks when mode changes
4. Measure Peltier element resistance (should be ~2-3Ω)
5. Check power supply voltage and current capacity

**Solutions:**
- Loose connection → Re-solder or re-crimp
- Dead ULN2003 → Replace IC
- Stuck relay → Replace relay
- Open Peltier → Replace Peltier element

### Problem 2: Temperature Overshoots

**Symptoms:** Temp goes past target by >2°C, oscillates

**Cause:** PID tuning too aggressive

**Solution:**
1. Reduce Kp (decrease proportional gain)
2. Increase Kd (increase damping)
3. Re-run auto-tune: `M303 E0 S20 C10`

### Problem 3: Temperature Slow to Respond

**Symptoms:** Takes >5 minutes to reach target

**Checks:**
1. Verify Peltier getting full power (check PWM duty cycle)
2. Check heat sink on cold side (may be saturated)
3. Verify thermal compound between Peltier and extruder

**Solutions:**
- Increase PELTIER_MAX_POWER values
- Add forced air cooling to cold side
- Increase Ki (faster integral response)

### Problem 4: Mode Oscillation

**Symptoms:** Rapidly switches between heating/cooling

**Cause:** Deadband too small or PID overshoot

**Solution:**
1. Increase E0_PELTIER_DEADBAND to 1.0°C
2. Reduce Ki (less aggressive integral action)
3. Add derivative filtering

### Problem 5: Thermal Runaway Error

**Symptoms:** Firmware shuts down with error message

**Checks:**
1. Verify thermistor connections (should read ~100kΩ at 25°C)
2. Check thermistor type setting (should be Type 1)
3. Verify timeout value is reasonable (5 minutes default)

**Solutions:**
- Loose thermistor → Re-seat connector
- Wrong thermistor type → Update Configuration.h
- Slow thermal response → Increase E0_PELTIER_TIMEOUT

---

## 9. Summary of Changes

### Files to Create:
- None (all modifications to existing files)

### Files to Modify:

| File | Lines Added | Purpose |
|------|-------------|---------|
| Configuration.h | ~30 | Peltier feature enable, PID params, temp limits |
| Configuration_adv.h | ~20 | Advanced Peltier settings |
| pins_BTT_OCTOPUS_V1_common.h | ~15 | Pin definitions for Peltier control |
| temperature.h | ~35 | Peltier class members and methods |
| temperature.cpp | ~200 | Peltier control logic and PID |
| M104.cpp | ~10 | G-code support for Peltier temp set |
| M109.cpp | ~25 | G-code support for Peltier temp wait |
| M105.cpp | ~5 | Temperature reporting for Peltier |
| menu_temperature.cpp | ~50 | LCD menu for Peltier control |
| language_en.h | ~10 | English language strings |

**Total Lines of Code:** ~400 lines

### Hardware Connections:

```
BTT Octopus        ULN2003         DPDT Relay       Peltier
-----------        -------         ----------       -------
PE5 (HEAT)    →    IN1       →    Relay 1    →    12V+ (Heat Mode)
PD13 (COOL)   →    IN2       →    Relay 2    →    12V+ (Cool Mode)
PD14 (POL)    →    IN3       →    DPDT Coil  →    Polarity Switch
GND           →    GND       →    GND
                   OUT1-7    →    Relay Coils
                   COM       →    GND

Thermistor:
PA3 (T0)      ←    NTC 100K  ←    Extruder
GND           ←    GND       ←    Extruder
```

---

## 10. Next Steps

### Implementation Order:

1. **Week 1: Hardware Assembly**
   - Wire DPDT relay and ULN2003
   - Connect Peltier element
   - Install thermistor
   - Test continuity and polarity

2. **Week 2: Firmware - Core**
   - Modify Configuration.h
   - Add pin definitions
   - Implement temperature.cpp core logic
   - Test basic on/off control

3. **Week 3: Firmware - PID**
   - Implement PID controller
   - Add mode switching logic
   - Test heating and cooling
   - Perform PID auto-tune

4. **Week 4: Integration**
   - Add G-code support (M104, M109, M105)
   - Create LCD menus
   - Add language strings
   - Full system testing

5. **Week 5: Testing & Calibration**
   - Safety testing
   - PID optimization
   - Temperature accuracy verification
   - Long-term stability testing

### Validation Checklist:

- [ ] Hardware wiring verified
- [ ] Firmware compiles without errors
- [ ] M104/M109 commands work
- [ ] M105 reports correct temperature
- [ ] LCD menu displays current/target temp
- [ ] Heating mode works (polarity correct)
- [ ] Cooling mode works (polarity reversed)
- [ ] PID auto-tune successful
- [ ] Temperature stays within ±1°C of target
- [ ] Safety timeout works
- [ ] MINTEMP/MAXTEMP protection works
- [ ] Slicer can set temperature via G-code
- [ ] No thermal runaway after 1 hour operation

---

## 11. References

### Datasheets:
- TEC1-12706 Peltier Element Datasheet
- ULN2003 Darlington Array Datasheet
- STM32F446 Reference Manual (for GPIO/PWM)
- BTT Octopus V1.1 Schematic

### Marlin Documentation:
- https://marlinfw.org/docs/configuration/configuration.html
- https://marlinfw.org/docs/gcode/M104.html (Set Hotend Temperature)
- https://marlinfw.org/docs/gcode/M109.html (Wait for Hotend Temperature)
- https://marlinfw.org/docs/gcode/M303.html (PID Autotune)

### PID Tuning Resources:
- https://en.wikipedia.org/wiki/PID_controller
- Ziegler-Nichols tuning method
- Manual PID tuning guide for Peltier systems

---

**Document Version:** 1.0
**Status:** Implementation Plan - Ready for Development
**Estimated Implementation Time:** 5 weeks
**Complexity Level:** Advanced
**Required Skills:** Firmware development, electronics, PID control theory
