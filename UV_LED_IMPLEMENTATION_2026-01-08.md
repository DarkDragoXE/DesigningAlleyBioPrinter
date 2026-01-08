# UV LED PWM Control Implementation
**Date:** 2026-01-08
**Feature:** UV LED control for photopolymer crosslinking
**Pin:** 61 (PD13 / FAN3_PIN on BTT Octopus V1.1)

---

## Overview

Implemented direct PWM control of UV LED array using M42 G-code commands for photopolymer bioprinting applications.

### Hardware Setup
- **Pin:** PD13 (FAN3_PIN, pin number 61)
- **Connection:** BTT Octopus → MOSFET (IRLZ44N) → UV LED Array
- **Control:** PWM signal (0-255) for intensity control
- **Default State:** OFF (LOW) at startup

---

## G-Code Commands

### Basic Control (M42 Direct Pin Control)

```gcode
M42 P61 S0     # UV LED OFF (0% intensity)
M42 P61 S64    # UV LED 25% intensity
M42 P61 S128   # UV LED 50% intensity
M42 P61 S191   # UV LED 75% intensity
M42 P61 S255   # UV LED 100% intensity (full power)
```

### Intensity Levels

| Command | PWM Value | Intensity | Use Case |
|---------|-----------|-----------|----------|
| `M42 P61 S0` | 0 | 0% (OFF) | No crosslinking, material handling |
| `M42 P61 S64` | 64 | 25% | Light crosslinking, thin layers |
| `M42 P61 S128` | 128 | 50% | Moderate crosslinking, standard layers |
| `M42 P61 S191` | 191 | 75% | Strong crosslinking, thick layers |
| `M42 P61 S255` | 255 | 100% | Maximum crosslinking, final cure |

---

## Usage Examples

### Example 1: Layer-by-Layer Crosslinking
```gcode
G1 X10 Y10 Z0.2 F1000      ; Move to position
G1 E5 F100                 ; Extrude photopolymer
M42 P61 S191               ; UV LED ON at 75%
G4 P3000                   ; Wait 3 seconds (cure time)
M42 P61 S0                 ; UV LED OFF
```

### Example 2: Progressive Intensity Cure
```gcode
G1 E10 F100                ; Extrude material
M42 P61 S64                ; Start at 25%
G4 P2000                   ; Wait 2 seconds
M42 P61 S128               ; Increase to 50%
G4 P2000                   ; Wait 2 seconds
M42 P61 S255               ; Maximum cure at 100%
G4 P3000                   ; Wait 3 seconds
M42 P61 S0                 ; UV LED OFF
```

### Example 3: Area Crosslinking
```gcode
M42 P61 S128               ; UV LED ON at 50%
G1 X0 Y0 F3000             ; Move to start
G1 X100 Y0                 ; Scan line 1
G1 X100 Y100               ; Scan line 2
G1 X0 Y100                 ; Scan line 3
G1 X0 Y0                   ; Return to start
M42 P61 S0                 ; UV LED OFF
```

---

## Files Modified

### 1. Configuration.h (Lines 157-164)
**Purpose:** Define UV LED custom pin for M42 control

```cpp
// BIOPRINTER: UV LED pin for photopolymer crosslinking via M42
// M42 P61 S0 = UV LED off (0%)
// M42 P61 S64 = UV LED 25% intensity
// M42 P61 S128 = UV LED 50% intensity
// M42 P61 S191 = UV LED 75% intensity
// M42 P61 S255 = UV LED 100% intensity (full power)
#define CUSTOM_UV_LED_PIN  61  // PD13 (FAN3_PIN)
#define UV_LED_CUSTOM_PIN_STATE LOW  // Start with UV LED off
```

**Why:**
- Uses CUSTOM_UV_LED_PIN naming convention similar to CUSTOM_BED_PIN
- Pin 61 (PD13/FAN3) is not in SENSITIVE_PINS, so M42 can control it without protection bypass
- Allows M42 P61 S0-255 commands to work without needing the `I` flag
- Initializes UV LED to OFF state at boot for safety

---

### 2. MarlinCore.cpp (Lines 322-325)
**Purpose:** Bypass pin protection for UV LED pin

```cpp
// BIOPRINTER: Allow M42 control of UV LED pin (used for photopolymer crosslinking)
#if CUSTOM_UV_LED_PIN
  if (pin == CUSTOM_UV_LED_PIN) return false;
#endif
```

**Why:**
- Pin 61 (PD13/FAN3_PIN) is in SENSITIVE_PINS array, causing "Protected Pin" error
- Explicit bypass for CUSTOM_UV_LED_PIN allows M42 P61 to work without `I` flag
- Matches the same approach used for CUSTOM_BED_PIN (pin 60)

---

### 3. MarlinCore.cpp (Lines 1178-1182)
**Purpose:** Initialize UV LED pin at startup

```cpp
// BIOPRINTER: Initialize UV LED pin (M42 P61 control for photopolymer crosslinking)
#if CUSTOM_UV_LED_PIN
  pinMode(CUSTOM_UV_LED_PIN, OUTPUT);
  digitalWrite(CUSTOM_UV_LED_PIN, UV_LED_CUSTOM_PIN_STATE);
#endif
```

**Why:**
- Sets pin 61 to OUTPUT mode early in boot sequence
- Forces pin to LOW (UV LED OFF) at startup for safety
- Prevents accidental UV exposure during firmware initialization
- Executed before main loop starts, ensuring pin is in known state

---

### 4. M42.cpp (Lines 93-96)
**Purpose:** Prevent fan interception for UV LED pin

```cpp
// BIOPRINTER: Allow direct control of CUSTOM_UV_LED_PIN even if it's a fan pin
#if CUSTOM_UV_LED_PIN
  if (pin != CUSTOM_UV_LED_PIN)
#endif
```

**Why:**
- M42.cpp has fan interception logic that intercepts fan pins (lines 94-119)
- Pin 61 = PD13 = FAN3_PIN, so M42 P61 would be intercepted without this fix
- Instead of setting GPIO directly, it would set `thermalManager.fan_speed[3]`
- This bypass ensures M42 P61 reaches `extDigitalWrite()` and `hal.set_pwm_duty()`
- Matches the same approach used for CUSTOM_BED_PIN (pin 60)

---

## Safety Features

### 1. Default OFF State
- UV LED starts in OFF state (LOW) at power-on
- Prevents accidental UV exposure during boot

### 2. Pin Protection Bypass
- Only CUSTOM_UV_LED_PIN is unprotected
- All other sensitive pins remain protected
- Prevents accidental control of critical pins

### 3. Manual Control
- UV LED is only controlled by explicit M42 commands
- No automatic activation
- User has full control over UV exposure timing and intensity

---

## Hardware Requirements

### Minimum Components
1. **MOSFET:** IRLZ44N (logic-level N-channel)
   - Gate Threshold: 1-2V (compatible with 3.3V logic)
   - Drain-Source Voltage: 55V
   - Continuous Drain Current: 30A

2. **Resistors:**
   - Gate resistor: 1kΩ (limits current to gate)
   - Pull-down resistor: 10kΩ (ensures MOSFET off when pin floating)

3. **Diode:** 1N4007 flyback diode
   - Protects MOSFET from inductive kickback

4. **UV LED Array:** 365nm or 405nm
   - Operating voltage: 12V or 24V
   - Power: 10-50W depending on application

### Wiring Diagram
```
BTT Octopus PD13 (Pin 61)
    |
    ├─── [1kΩ Resistor] ─── MOSFET Gate
    |
    └─── [10kΩ to GND]      (pull-down)

MOSFET Source ─── Common GND (shared with BTT Octopus)

MOSFET Drain ─── UV LED (-)

UV LED (+) ─── UV Power Supply (+12V or +24V)

UV Power Supply (-) ─── Common GND

[1N4007 Diode across UV LED]: Cathode to (+), Anode to (-)
```

### Power Supply Requirements
- **Voltage:** Match UV LED array voltage (typically 12V or 24V)
- **Current:** Sufficient for UV LED array (10-50W → 1-5A at 12V)
- **Ground:** MUST share common ground with BTT Octopus

---

## Testing Procedures

### Test 1: Pin Initialization
**Expected:** UV LED should be OFF at power-on

1. Connect multimeter to PD13 (pin 61)
2. Power on BTT Octopus
3. Verify voltage = 0V (LOW)

**Status:** ✅ Should pass with current implementation

---

### Test 2: Direct PWM Control
**Expected:** M42 commands control UV LED intensity

```gcode
M42 P61 S0      ; Verify 0V (LED off)
M42 P61 S64     ; Verify ~0.8V average (25% duty cycle)
M42 P61 S128    ; Verify ~1.6V average (50% duty cycle)
M42 P61 S191    ; Verify ~2.5V average (75% duty cycle)
M42 P61 S255    ; Verify 3.3V (100%, full on)
M42 P61 S0      ; Return to off
```

**Measurement:**
- Use multimeter in DC voltage mode
- Measure voltage at PD13 (pin 61) relative to ground
- PWM averages: 0V, 0.8V, 1.6V, 2.5V, 3.3V

---

### Test 3: MOSFET Switching
**Expected:** MOSFET switches UV LED array on/off

1. Connect UV LED array to MOSFET circuit
2. Send M42 P61 S255 (UV LED should turn on)
3. Verify UV LED is ON (visually or with UV sensor)
4. Send M42 P61 S0 (UV LED should turn off)
5. Verify UV LED is OFF

**Safety:** Wear UV-blocking safety glasses during this test

---

### Test 4: PWM Intensity Control
**Expected:** Different intensities produce different brightness

```gcode
M42 P61 S64     ; 25% - dim
M42 P61 S128    ; 50% - medium
M42 P61 S191    ; 75% - bright
M42 P61 S255    ; 100% - full brightness
M42 P61 S0      ; OFF
```

**Measurement:**
- Use UV light meter or photodiode
- Verify relative intensities: 25% < 50% < 75% < 100%

---

### Test 5: Bioprinting Integration
**Expected:** UV crosslinking works during print

```gcode
G28                        ; Home all axes
G1 Z0.2 F500              ; Move to first layer height
G1 X50 Y50 F3000          ; Move to center
G1 E5 F100                ; Extrude photopolymer
M42 P61 S191              ; UV ON at 75%
G4 P3000                  ; Wait 3 seconds
M42 P61 S0                ; UV OFF
G1 E-1 F100               ; Retract
```

**Verification:**
- Material should be cured after UV exposure
- Check adhesion, hardness, and layer bonding

---

## Troubleshooting

### Issue 1: UV LED Not Turning On
**Symptoms:** M42 P61 S255 sent, but LED doesn't turn on

**Possible Causes:**
1. **Pin voltage not reaching gate:** Check voltage at PD13 with multimeter
2. **MOSFET not conducting:** Check MOSFET with continuity tester
3. **Power supply issue:** Verify UV power supply is ON and correct voltage
4. **Wiring error:** Verify all connections match wiring diagram
5. **Ground not common:** Ensure BTT Octopus GND = UV PSU GND

**Solutions:**
- Verify PD13 outputs 3.3V when M42 P61 S255 is sent
- Check MOSFET gate threshold voltage (should be < 2V for IRLZ44N)
- Verify UV power supply voltage with multimeter
- Test MOSFET by connecting gate directly to 3.3V

---

### Issue 2: UV LED Always On
**Symptoms:** UV LED stays on regardless of M42 commands

**Possible Causes:**
1. **Pin stuck HIGH:** Initialization failed or hardware short
2. **MOSFET gate floating:** Missing pull-down resistor
3. **Wiring short:** Gate and drain shorted together

**Solutions:**
- Send M42 P61 S0 and verify PD13 = 0V
- Add 10kΩ pull-down resistor from gate to ground
- Check for shorts between gate and drain

---

### Issue 3: PWM Not Working (Only On/Off)
**Symptoms:** S64, S128, S191 produce same brightness as S255

**Possible Causes:**
1. **PWM not enabled on pin:** STM32 pin may not support PWM
2. **hal.set_pwm_duty() failing:** Check firmware PWM configuration

**Solutions:**
- Verify PD13 (TIM4_CH2) supports PWM in STM32F446 datasheet
- Check M42.cpp line 139: `hal.set_pwm_duty(pin, pin_status)`
- Test with oscilloscope to verify PWM signal frequency and duty cycle

---

### Issue 4: "Protected Pin" Error
**Symptoms:** `M42 P61 S255` returns "Error: Protected pin"

**Possible Causes:**
1. **Protection bypass not working:** CUSTOM_UV_LED_PIN not defined
2. **Firmware not rebuilt:** Changes not compiled

**Solutions:**
- Verify Configuration.h line 163: `#define CUSTOM_UV_LED_PIN 61`
- Rebuild firmware: `pio run -e STM32F446ZE_btt`
- Flash updated firmware to BTT Octopus
- Verify with `M42 P61 S255` (should work without error)

---

## PWM Technical Details

### How PWM Works
- **PWM = Pulse Width Modulation**
- Rapidly switches pin between 0V (LOW) and 3.3V (HIGH)
- **Duty Cycle:** Percentage of time pin is HIGH vs LOW
- **Frequency:** ~490 Hz (too fast for human eye to see flicker)

### Duty Cycle Calculation
```
Duty Cycle (%) = (PWM Value / 255) × 100%

S0   →   0/255 = 0%    → Always LOW (0V)
S64  →  64/255 = 25%   → HIGH 25% of time (0.8V average)
S128 → 128/255 = 50%   → HIGH 50% of time (1.6V average)
S191 → 191/255 = 75%   → HIGH 75% of time (2.5V average)
S255 → 255/255 = 100%  → Always HIGH (3.3V)
```

### Why PWM Controls Brightness
- UV LED doesn't perceive "average voltage" - it sees ON/OFF cycles
- At 490 Hz, LED turns on/off 490 times per second
- Human eye can't see flicker above ~60 Hz
- **Perceived brightness** is proportional to average power = duty cycle

### MOSFET Operation
- MOSFET gate voltage > threshold (1-2V) → MOSFET conducts → LED ON
- MOSFET gate voltage < threshold → MOSFET off → LED OFF
- PWM signal switches MOSFET on/off rapidly
- Average current through LED = duty cycle × max current

---

## Future Enhancements (Not Implemented)

### Option 1: Custom M355 Command
Create dedicated UV LED command for professional use:
```gcode
M355 S1 P255   ; UV LED on, 100%
M355 S1 P128   ; UV LED on, 50%
M355 S0        ; UV LED off
```

**Benefits:**
- Industry-standard command (used in some 3D printers)
- Cleaner G-code (M355 instead of M42 P61)
- Can add additional safety features

**Drawbacks:**
- Requires custom firmware code
- More complex to implement
- Current M42 method works fine

---

### Option 2: Fan Mapping (M106/M107)
Map UV LED to fan commands for slicer compatibility:
```gcode
M106 P2 S255   ; UV LED 100% (uses fan 2 slot)
M106 P2 S128   ; UV LED 50%
M107 P2        ; UV LED off
```

**Benefits:**
- Easy slicer integration (PrusaSlicer, Cura)
- Can add to layer change scripts
- Familiar commands for users

**Drawbacks:**
- Confusing (UV LED appears as "fan" in slicers)
- Requires FAN_COUNT configuration changes
- Current M42 method is more explicit

---

## Next Steps

1. **Build Firmware:**
   ```bash
   cd c:\BIOPRINTER\BTTOctopusDebKeshava\OctopusMarlin-bugfix-test
   pio run -e STM32F446ZE_btt
   ```

2. **Flash Firmware:**
   - Copy `.pio\build\STM32F446ZE_btt\firmware.bin` to SD card
   - Insert SD card into BTT Octopus
   - Reset board (firmware flashes automatically)

3. **Test UV LED Control:**
   ```gcode
   M42 P61 S0      ; Verify OFF
   M42 P61 S255    ; Verify ON
   M42 P61 S128    ; Verify 50%
   M42 P61 S0      ; Return to OFF
   ```

4. **Build MOSFET Circuit:**
   - Follow wiring diagram above
   - Use IRLZ44N MOSFET, 1kΩ and 10kΩ resistors, 1N4007 diode
   - Connect UV LED array to 12V/24V power supply

5. **Test with Photopolymer:**
   - Load photopolymer bioink
   - Extrude small test line
   - Expose with UV LED at different intensities
   - Verify crosslinking and cure quality

---

## Summary

### Changes Made
1. ✅ Added CUSTOM_UV_LED_PIN definition (Configuration.h)
2. ✅ Bypassed pin protection for pin 61 (MarlinCore.cpp)
3. ✅ Added UV LED initialization at startup (MarlinCore.cpp)
4. ✅ Prevented fan interception for pin 61 (M42.cpp)

### Status
- **Firmware:** Modified, needs rebuild and flash
- **Hardware:** Not yet connected (MOSFET circuit required)
- **Testing:** Pending after firmware flash

### G-Code Commands Ready
```gcode
M42 P61 S0      # OFF
M42 P61 S64     # 25%
M42 P61 S128    # 50%
M42 P61 S191    # 75%
M42 P61 S255    # 100%
```

---

**End of Implementation Report**
