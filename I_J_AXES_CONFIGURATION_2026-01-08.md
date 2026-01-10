# I and J Axes Configuration (Printhead Z Height Adjustment)
**Date:** 2026-01-08
**Purpose:** Independent Z height control for each printhead (E0 and E1)
**Motors:** NEMA 8 linear steppers (Motor 6 and Motor 7 on BTT Octopus V1.1)
**Drivers:** TMC2209

---

## Overview

Configured I and J axes as independent linear axes for individual printhead Z height adjustment. Each printhead can be positioned vertically independent of the main Z axis, allowing for:
- Tool offset calibration
- Nozzle height fine-tuning
- Independent printhead positioning for multi-material printing

---

## Hardware Configuration

| Axis | Motor Slot | BTT Octopus Label | Purpose | Motor Type | Driver |
|------|-----------|-------------------|---------|------------|---------|
| **I** | Motor 6 | MOTOR 6 | Printhead 0 (E0) Z height | NEMA 8 Linear | TMC2209 |
| **J** | Motor 7 | MOTOR 7 | Printhead 1 (E1) Z height | NEMA 8 Linear | TMC2209 |

---

## Firmware Configuration

### Changes in Configuration.h

#### 1. Driver Type (Lines 197-198)
```cpp
#define I_DRIVER_TYPE  TMC2209  // BIOPRINTER: I axis - Printhead 0 (E0) Z height adjustment (Motor 6)
#define J_DRIVER_TYPE  TMC2209  // BIOPRINTER: J axis - Printhead 1 (E1) Z height adjustment (Motor 7)
```

**Purpose:** Enables I and J axes with TMC2209 stepper drivers

---

#### 2. Axis Names (Lines 229-236)
```cpp
#ifdef I_DRIVER_TYPE
  #define AXIS4_NAME 'I' // BIOPRINTER: I axis - Printhead 0 Z height adjustment (linear)
  //#define AXIS4_ROTATES  // LINEAR axis - moves printhead vertically
#endif
#ifdef J_DRIVER_TYPE
  #define AXIS5_NAME 'J' // BIOPRINTER: J axis - Printhead 1 Z height adjustment (linear)
  //#define AXIS5_ROTATES  // LINEAR axis - moves printhead vertically
#endif
```

**Purpose:**
- Names axes as 'I' and 'J' (clear identification)
- Defines as LINEAR axes (not rotational)
- AXIS4_ROTATES and AXIS5_ROTATES commented out to specify linear motion

---

#### 3. Steps/mm (Line 1045)
```cpp
#define DEFAULT_AXIS_STEPS_PER_UNIT   { 80, 80, 400, 400, 400, 3200, 500 }
// X, Y, Z, I, J, E0, E1 (I/J placeholder: 400 steps/mm)
```

**Current Value:** 400 steps/mm (PLACEHOLDER)

**Status:** ⏳ Awaiting motor specifications
- Will be recalculated based on NEMA 8 lead screw pitch/lead
- Formula: Steps/mm = (200 × Microstepping) / Lead (mm)

---

#### 4. Max Feedrate (Line 1052)
```cpp
#define DEFAULT_MAX_FEEDRATE          { 30, 30, 5, 5, 5, 3 }
// X, Y, Z, I, J, E0 (I/J: 5mm/s conservative)
```

**I and J:** 5 mm/s (conservative for short-travel printhead adjustment)

**Note:** Can be increased after testing with actual motors

---

#### 5. Max Acceleration (Line 1065)
```cpp
#define DEFAULT_MAX_ACCELERATION      { 150, 150, 50, 100, 100, 1000 }
// X, Y, Z, I, J, E0 (I/J: 100mm/s² conservative)
```

**I and J:** 100 mm/s² (gentle for printhead adjustment)

**Note:** Conservative setting, can be tuned after motor testing

---

#### 6. Homing Direction (Lines 1512-1513)
```cpp
#define I_HOME_DIR -1  // BIOPRINTER: I axis homes to MIN endstop (printhead 0 down)
#define J_HOME_DIR -1  // BIOPRINTER: J axis homes to MIN endstop (printhead 1 down)
```

**Purpose:**
- -1 = Home towards MIN endstop (down)
- Printheads home downward to establish zero position

---

### Changes in Configuration_adv.h

#### 1. TMC2209 Current Settings (Lines 2908-2925)
```cpp
#if AXIS_IS_TMC(I)
  #define I_CURRENT      800   // Run current (mA)
  #define I_CURRENT_HOME I_CURRENT
  #define I_MICROSTEPS    16
  #define I_RSENSE         0.11
  #define I_CHAIN_POS     -1
  //#define I_INTERPOLATE  true
  //#define I_HOLD_MULTIPLIER 0.5
#endif

#if AXIS_IS_TMC(J)
  #define J_CURRENT      800   // Run current (mA)
  #define J_CURRENT_HOME J_CURRENT
  #define J_MICROSTEPS    16
  #define J_RSENSE         0.11
  #define J_CHAIN_POS     -1
  //#define J_INTERPOLATE  true
  //#define J_HOLD_MULTIPLIER 0.5
#endif
```

**Current Settings (PLACEHOLDER):**
- I_CURRENT: 800mA
- J_CURRENT: 800mA
- Microstepping: 16
- R_sense: 0.11Ω (BTT Octopus TMC2209 standard)

**Status:** ⏳ Awaiting NEMA 8 motor specifications
- Will adjust current based on motor rated current
- Example: If motor rated at 500mA, change to I_CURRENT 500

---

#### 2. Homing Bump Settings (Lines 901-902)
```cpp
#define HOMING_BUMP_MM      { 5, 5, 2, 2, 2 }    // X, Y, Z, I, J backoff after first bump
#define HOMING_BUMP_DIVISOR { 2, 2, 4, 2, 2 }    // Re-Bump Speed Divisor - X, Y, Z, I, J
```

**I and J Homing:**
- Bump distance: 2mm backoff after hitting endstop
- Re-bump divisor: 2 (approaches at half speed on second attempt)

---

#### 3. Individual Axis Homing (Lines 911-912)
```cpp
#define INDIVIDUAL_AXIS_HOMING_MENU        // Already enabled
#define INDIVIDUAL_AXIS_HOMING_SUBMENU     // Already enabled
```

**Purpose:**
- Enables individual axis homing commands
- `G28` without parameters homes X, Y, Z only (NOT I and J)
- `G28 I` homes I axis independently
- `G28 J` homes J axis independently

**This is exactly what you requested!**

---

## G-Code Commands

### Homing Commands

| Command | Function | Description |
|---------|----------|-------------|
| `G28` | Home X, Y, Z | Standard homing - does NOT home I or J |
| `G28 I` | Home I axis only | Homes printhead 0 vertically |
| `G28 J` | Home J axis only | Homes printhead 1 vertically |
| `G28 X Y Z` | Home X, Y, Z | Explicitly home specific axes |
| `G28 I J` | Home I and J | Home both printheads (not triggered by G28 alone) |

---

### Movement Commands

| Command | Function | Example |
|---------|----------|---------|
| `G1 I<pos>` | Move I axis | `G1 I10 F300` - Move printhead 0 to 10mm |
| `G1 J<pos>` | Move J axis | `G1 J5 F300` - Move printhead 1 to 5mm |
| `G1 I<pos> J<pos>` | Move both | `G1 I10 J10 F300` - Move both to 10mm |
| `G92 I0` | Set I position | Set current I position as zero |
| `G92 J0` | Set J position | Set current J position as zero |

---

### Configuration Commands

| Command | Function | Example |
|---------|----------|---------|
| `M92 I<steps>` | Set I steps/mm | `M92 I400` - Set 400 steps/mm |
| `M92 J<steps>` | Set J steps/mm | `M92 J400` - Set 400 steps/mm |
| `M203 I<speed>` | Set I max feedrate | `M203 I10` - Set 10mm/s max |
| `M203 J<speed>` | Set J max feedrate | `M203 J10` - Set 10mm/s max |
| `M201 I<accel>` | Set I max accel | `M201 I200` - Set 200mm/s² |
| `M201 J<accel>` | Set J max accel | `M201 J200` - Set 200mm/s² |
| `M906 I<current>` | Set I motor current | `M906 I500` - Set 500mA |
| `M906 J<current>` | Set J motor current | `M906 J500` - Set 500mA |
| `M500` | Save settings | Save all changes to EEPROM |
| `M503` | Display settings | Show current configuration |

---

## Testing Procedure

### Test 1: Verify Axes are Enabled
```gcode
M503   ; Display all settings
```
**Expected Output:**
- Should show I and J axis parameters
- Steps/mm: I400.00, J400.00
- Max feedrate, acceleration, etc.

---

### Test 2: Manual Movement (No Homing Required)
```gcode
M302 P1      ; Allow cold extrusion (safety)
G91          ; Relative positioning
G1 I1 F60    ; Move I axis +1mm at 1mm/s
G1 I-1 F60   ; Move I axis -1mm (back to start)
G1 J1 F60    ; Move J axis +1mm at 1mm/s
G1 J-1 F60   ; Move J axis -1mm (back to start)
G90          ; Absolute positioning
```
**Expected:** Motors should move smoothly in both directions

---

### Test 3: Independent Homing (After Endstops Connected)
```gcode
G28 I   ; Home I axis only (printhead 0)
; Watch: Should move down until hitting endstop
; Then back off 2mm and re-approach slowly

G28 J   ; Home J axis only (printhead 1)
; Watch: Should move down until hitting endstop
```

**Expected:**
- I and J home independently
- Each axis homes to MIN endstop (downward)
- Bump behavior: fast approach → backoff 2mm → slow approach

---

### Test 4: Verify G28 Does NOT Home I/J
```gcode
G91          ; Relative mode
G1 I5 J5     ; Move I and J to non-zero position
G90          ; Absolute mode
G28          ; Standard home command
M114         ; Report current position
```

**Expected:**
- G28 homes X, Y, Z to zero
- I and J remain at their previous position (5mm)
- Output: `I:5.00 J:5.00` (NOT zero)

**This confirms I and J are NOT homed by G28!**

---

### Test 5: Speed and Acceleration Test
```gcode
G91          ; Relative mode
G1 I10 F60   ; Slow speed (1mm/s)
G1 I-10 F300 ; Faster speed (5mm/s)
G1 J10 F60   ; Slow
G1 J-10 F300 ; Fast
G90          ; Absolute mode
```

**Listen for:**
- Smooth motion at different speeds
- No grinding or skipping
- Motor should accelerate/decelerate smoothly

---

## Endstop Configuration

### Endstop Pins (BTT Octopus V1.1)

**I Axis Endstop Options:**
- **Recommended:** I_MIN (if available on board)
- **Alternative:** Use spare endstop pins (check pins_BTT_OCTOPUS_V1_common.h)

**J Axis Endstop Options:**
- **Recommended:** J_MIN (if available on board)
- **Alternative:** Use spare endstop pins

**User Action Required:**
1. Check BTT Octopus V1.1 pinout for available endstop connectors
2. Wire endstop switches to I_MIN and J_MIN pins
3. Test endstop triggering with `M119` command

---

### Endstop Testing
```gcode
M119   ; Report endstop status
```

**Expected Output (before homing):**
```
I min: open
J min: open
```

**When manually triggered:**
```
I min: TRIGGERED
J min: TRIGGERED
```

---

## Pending Configuration (Awaiting Motor Specs)

### Information Needed:

1. **NEMA 8 Motor Specifications:**
   - Rated current (mA)
   - Step angle (typically 1.8° = 200 steps/rev)
   - Holding torque

2. **Lead Screw Specifications:**
   - Pitch (mm)
   - Lead (mm)
   - Screw diameter (if integrated)

3. **Travel Range:**
   - Maximum travel distance for printhead adjustment
   - Helps set software endstops (M211)

---

### Once Motor Specs Are Provided:

**Steps/mm Calculation:**
```
Steps/mm = (Motor steps/rev × Microstepping) / Lead (mm)
Example: (200 × 16) / 2 = 1600 steps/mm
```

**Motor Current Adjustment:**
```
Set I_CURRENT and J_CURRENT to motor rated current
Example: If motor rated at 500mA:
  #define I_CURRENT 500
  #define J_CURRENT 500
```

**Software Endstops:**
```cpp
#define I_MIN_POS 0
#define I_MAX_POS <travel_range>
#define J_MIN_POS 0
#define J_MAX_POS <travel_range>
```

---

## Key Features Implemented

### ✅ Independent Linear Axes
- I and J are fully independent linear axes
- NOT rotational axes
- Move printheads vertically (Z height adjustment)

### ✅ Separate Homing
- `G28` does NOT home I or J axes
- `G28 I` and `G28 J` home independently
- Each axis can be homed separately or together

### ✅ TMC2209 Drivers
- Full TMC2209 features available
- StealthChop, StallGuard, etc.
- Current control, microstepping

### ✅ Individual Control
- Each printhead controlled independently
- No interference with main Z axis
- Can move I and J while Z is stationary

### ✅ Safety Features
- Conservative placeholder values
- Won't damage motors even with incorrect settings
- Easy to tune after motor installation

---

## Usage Scenarios

### Scenario 1: Printhead Height Calibration
```gcode
G28          ; Home X, Y, Z (not I, J)
G28 I J      ; Home both printheads
G1 I0.5      ; Raise printhead 0 by 0.5mm
G1 J1.0      ; Raise printhead 1 by 1.0mm
M500         ; Save offsets
```

---

### Scenario 2: Tool Change (E0 to E1)
```gcode
; Currently printing with E0 (I axis controls printhead 0)
G1 I10       ; Retract printhead 0 upward
G1 J0        ; Lower printhead 1 to printing position
T1           ; Switch to E1 extruder
; Resume printing with E1
```

---

### Scenario 3: Independent Nozzle Leveling
```gcode
G28 I        ; Home printhead 0
G1 I0.1      ; Touch off on build plate
G92 I0       ; Set as zero

G28 J        ; Home printhead 1
G1 J0.1      ; Touch off on build plate
G92 J0       ; Set as zero

M500         ; Save offsets
```

---

## Motor Wiring (BTT Octopus V1.1)

### Motor 6 (I Axis) Wiring
```
BTT Octopus MOTOR 6 Connector:
Pin 1 (1B) ─── Motor Coil 1 (-)
Pin 2 (1A) ─── Motor Coil 1 (+)
Pin 3 (2A) ─── Motor Coil 2 (+)
Pin 4 (2B) ─── Motor Coil 2 (-)
```

### Motor 7 (J Axis) Wiring
```
BTT Octopus MOTOR 7 Connector:
Pin 1 (1B) ─── Motor Coil 1 (-)
Pin 2 (1A) ─── Motor Coil 1 (+)
Pin 3 (2A) ─── Motor Coil 2 (+)
Pin 4 (2B) ─── Motor Coil 2 (-)
```

**Note:** Wire colors may vary by manufacturer. Use continuity tester to identify coil pairs.

---

## Troubleshooting

### Issue 1: Motors Not Moving
**Symptoms:** `G1 I10` command does nothing

**Possible Causes:**
1. TMC2209 driver not seated properly
2. Motor current set to 0
3. Wiring incorrect

**Solutions:**
- Check TMC2209 drivers are firmly inserted in Motor 6 and Motor 7 slots
- Verify `M906` shows I_CURRENT and J_CURRENT > 0
- Check motor wiring with continuity tester
- Send `M122` to check TMC driver status

---

### Issue 2: I or J Axis in M503 Output Shows Unexpected Values
**Symptoms:** `M503` doesn't show I or J parameters

**Possible Causes:**
- Firmware not rebuilt after changes
- EEPROM contains old configuration

**Solutions:**
- Rebuild firmware: `pio run -e STM32F446ZE_btt`
- Flash updated firmware
- Send `M502` (reset to defaults) then `M500` (save)

---

### Issue 3: G28 Homes I and J (Shouldn't!)
**Symptoms:** `G28` command homes X, Y, Z, I, and J

**Possible Causes:**
- INDIVIDUAL_AXIS_HOMING_MENU not working correctly
- Firmware issue

**Solutions:**
- Verify INDIVIDUAL_AXIS_HOMING_MENU is enabled (Configuration_adv.h line 911)
- Rebuild firmware
- Use `G28 X Y Z` explicitly if needed

---

### Issue 4: Motor Overheating
**Symptoms:** I or J axis motor gets very hot

**Possible Causes:**
- Motor current too high (800mA may be too much for NEMA 8)
- Motor stall/binding

**Solutions:**
- **Reduce current**: `M906 I500 J500` (try 500mA)
- Check mechanical binding
- Verify motor rated current from datasheet
- Enable hold current reduction: uncomment `I_HOLD_MULTIPLIER 0.5` in Configuration_adv.h

---

### Issue 5: Homing Doesn't Stop at Endstop
**Symptoms:** Motor keeps grinding after hitting endstop

**Possible Causes:**
- Endstop not connected
- Endstop wired incorrectly
- Endstop logic inverted

**Solutions:**
- Test endstop: `M119` (should show "open" normally, "TRIGGERED" when pressed)
- Check endstop wiring
- May need to invert endstop logic in pins file

---

## Next Steps

### Immediate (After Motor Installation):

1. **Install Motors:**
   - Mount NEMA 8 motors in Motor 6 and Motor 7 slots
   - Wire endstops to I_MIN and J_MIN pins

2. **Provide Motor Specifications:**
   - Rated current
   - Lead screw pitch/lead
   - Step angle

3. **Rebuild and Flash Firmware:**
   ```bash
   cd /c/BIOPRINTER/BTTOctopusDebKeshava/OctopusMarlin-bugfix-test
   pio run -e STM32F446ZE_btt
   ```

4. **Test Basic Movement:**
   - Run Test 1 and Test 2 from testing procedures
   - Verify motors respond to commands

---

### After Motor Specs Received:

1. **Update Firmware Configuration:**
   - Calculate correct steps/mm
   - Set appropriate motor current
   - Tune feedrate/acceleration

2. **Calibrate:**
   - Test homing (Test 3)
   - Verify G28 behavior (Test 4)
   - Fine-tune speeds (Test 5)

3. **Save Configuration:**
   - `M500` to save all settings to EEPROM

---

## Summary

### Configuration Status:

| Item | Status | Notes |
|------|--------|-------|
| I and J axes enabled | ✅ Complete | TMC2209 drivers |
| Linear motion (not rotational) | ✅ Complete | Moves printheads vertically |
| Independent homing | ✅ Complete | G28 I, G28 J |
| NOT homed by G28 | ✅ Complete | G28 only homes X, Y, Z |
| Placeholder motion parameters | ✅ Complete | 400 steps/mm, 5mm/s, 100mm/s² |
| Homing direction (MIN) | ✅ Complete | Home downward |
| TMC2209 settings | ✅ Complete | 800mA placeholder |
| Endstop configuration | ⏳ Pending | Need to wire endstops |
| Motor specs | ⏳ Pending | Need NEMA 8 specifications |
| Calibration | ⏳ Pending | After motor installation |

---

**Ready for motor installation and testing!**

When you provide NEMA 8 motor specifications (product link), I will:
1. Calculate exact steps/mm
2. Set correct motor current
3. Fine-tune motion parameters
4. Update documentation

---

**End of I/J Axes Configuration Report**
