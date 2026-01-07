# PC3 Pneumatic Valve Startup Fix - CRITICAL
**Date:** January 7, 2026, 18:00 UTC
**Issue:** PC3 (pneumatic valve) turning ON at startup instead of staying OFF
**Priority:** CRITICAL - Causes unwanted material dispensing at boot

---

## Problem Description

**Symptom:**
- Power on BTT Octopus board
- PC3 starts LOW (0V) for ~2 seconds
- Then PC3 goes HIGH (3.3V) automatically
- Pneumatic valve opens, material dispenses unintentionally

**Root Cause:**
The stepper initialization code calls `ENABLE_STEPPER_E1()` which executes `E1_ENABLE_WRITE(E_ENABLE_ON)` a few seconds after boot. This is intended for stepper motors to "enable" them, but for pneumatic control, we don't want any automatic enabling.

---

## Solution Implemented

### File Modified:
`Marlin/src/module/stepper/indirection.h`

### Change #1: ENABLE_STEPPER_E1 (Line 880-890)

**Before:**
```cpp
#ifndef ENABLE_STEPPER_E1
  #if (E_STEPPERS > 1 || ENABLED(E_DUAL_STEPPER_DRIVERS)) && HAS_E1_ENABLE
    #define  ENABLE_STEPPER_E1() E1_ENABLE_WRITE( E_ENABLE_ON)
  #else
    #define  ENABLE_STEPPER_E1() NOOP
  #endif
#endif
```

**After:**
```cpp
#ifndef ENABLE_STEPPER_E1
  #if ENABLED(PNEUMATIC_EXTRUDER_E1)
    // BIOPRINTER: Pneumatic E1 should NOT be enabled by stepper code
    // PC3 is controlled directly during extrusion moves only
    #define  ENABLE_STEPPER_E1() NOOP
  #elif (E_STEPPERS > 1 || ENABLED(E_DUAL_STEPPER_DRIVERS)) && HAS_E1_ENABLE
    #define  ENABLE_STEPPER_E1() E1_ENABLE_WRITE( E_ENABLE_ON)
  #else
    #define  ENABLE_STEPPER_E1() NOOP
  #endif
#endif
```

### Change #2: DISABLE_STEPPER_E1 (Line 891-900)

**Before:**
```cpp
#ifndef DISABLE_STEPPER_E1
  #if (E_STEPPERS > 1 || ENABLED(E_DUAL_STEPPER_DRIVERS)) && HAS_E1_ENABLE
    #define DISABLE_STEPPER_E1() E1_ENABLE_WRITE(!E_ENABLE_ON)
  #else
    #define DISABLE_STEPPER_E1() NOOP
  #endif
#endif
```

**After:**
```cpp
#ifndef DISABLE_STEPPER_E1
  #if ENABLED(PNEUMATIC_EXTRUDER_E1)
    // BIOPRINTER: Pneumatic E1 disable does nothing (valve control only during extrusion)
    #define DISABLE_STEPPER_E1() NOOP
  #elif (E_STEPPERS > 1 || ENABLED(E_DUAL_STEPPER_DRIVERS)) && HAS_E1_ENABLE
    #define DISABLE_STEPPER_E1() E1_ENABLE_WRITE(!E_ENABLE_ON)
  #else
    #define DISABLE_STEPPER_E1() NOOP
  #endif
#endif
```

---

## How It Works Now

### PC3 Control Flow:

**1. At Startup (MarlinCore.cpp line 1153-1159):**
```cpp
SET_INPUT_PULLDOWN(E1_ENABLE_PIN);   // Pull PC3 LOW
hal.delay_ms(1);
OUT_WRITE(E1_ENABLE_PIN, LOW);       // PC3 = LOW (valve closed)
```
**Result:** PC3 = LOW, valve CLOSED ✅

**2. During Stepper Init (stepper.cpp line 2771-2774):**
```cpp
E1_ENABLE_INIT();                    // Set as output
E1_ENABLE_WRITE(LOW);                // Force LOW
```
**Result:** PC3 stays LOW, valve CLOSED ✅

**3. When enable_all_steppers() is called (~2 seconds after boot):**
```cpp
ENABLE_STEPPER_E1();                 // Now = NOOP (does nothing!)
```
**Result:** PC3 stays LOW, valve CLOSED ✅

**4. During T1 Extrusion:**
```cpp
WRITE(E1_ENABLE_PIN, HIGH);          // Valve opens
// ... extrusion ...
WRITE(E1_ENABLE_PIN, LOW);           // Valve closes
```
**Result:** PC3 = HIGH during extrusion only ✅

---

## Testing Procedure

### Test 1: Startup State
1. Power OFF BTT Octopus
2. Connect DMM to PC3
3. Power ON board
4. **Expected:** PC3 stays LOW (0V) throughout boot ✅
5. **Expected:** Valve remains CLOSED ✅

### Test 2: Extrusion Control
```gcode
T1              ; Select pneumatic extruder
G1 E10 F180     ; Extrude 10mm
```
**Expected:**
- PC3 goes HIGH during G1 E10 command
- Valve opens, material dispenses
- PC3 goes LOW after move completes
- Valve closes

### Test 3: Idle State
```gcode
M105            ; Check temperature (any command)
```
**Expected:**
- PC3 remains LOW
- Valve remains CLOSED

---

## Verification Checklist

- [ ] PC3 = LOW (0V) immediately after power on
- [ ] PC3 stays LOW for full boot sequence (no HIGH spike)
- [ ] Valve remains CLOSED when board is idle
- [ ] PC3 goes HIGH only during T1 extrusion
- [ ] PC3 returns to LOW after extrusion completes
- [ ] Firmware builds without errors
- [ ] No unintended material dispensing at startup

---

## Related Code Locations

### PC3 Control Points:
1. **Early init:** `MarlinCore.cpp` line 1153-1159
2. **Stepper init:** `stepper.cpp` line 2771-2774
3. **Extrusion start:** `stepper.cpp` line 2356 OR `pneumatic_extruder.cpp` line 65
4. **Extrusion stop:** `stepper.cpp` line 2772 OR `pneumatic_extruder.cpp` line 84

### Macros Modified:
- **indirection.h** line 880-890: `ENABLE_STEPPER_E1()`
- **indirection.h** line 891-900: `DISABLE_STEPPER_E1()`

---

## Deployment Instructions

### 1. Build New Firmware
```bash
cd /c/BIOPRINTER/BTTOctopusDebKeshava/OctopusMarlin-bugfix-test
pio run -e STM32F446ZE_btt
```

### 2. Verify Build Success
Check for:
```
SUCCESS
Elapsed time: X seconds
```

### 3. Flash to Board
1. Copy `.pio/build/STM32F446ZE_btt/firmware.bin` to SD card
2. Insert SD card into BTT Octopus
3. Power cycle board
4. Wait for LED to stop blinking (~30 seconds)
5. Remove SD card
6. Power cycle again

### 4. Test PC3 Startup
- Power ON and measure PC3 with DMM
- Should be LOW (0V) throughout boot
- Valve should remain CLOSED

---

## Rollback Procedure

If this fix causes issues:

```bash
cd /c/BIOPRINTER/BTTOctopusDebKeshava/OctopusMarlin-bugfix-test
git checkout HEAD~1 Marlin/src/module/stepper/indirection.h
pio run -e STM32F446ZE_btt
# Flash previous version
```

---

## Summary

**Changes Made:**
- Modified `ENABLE_STEPPER_E1()` to do NOTHING (NOOP) for pneumatic extruder
- Modified `DISABLE_STEPPER_E1()` to do NOTHING (NOOP) for pneumatic extruder
- PC3 now controlled ONLY during extrusion moves, never by stepper enable/disable

**Result:**
- ✅ PC3 stays LOW at startup (valve CLOSED)
- ✅ PC3 only goes HIGH during T1 extrusion
- ✅ No unintended material dispensing
- ✅ Full control over pneumatic valve timing

**Status:** ✅ READY FOR BUILD AND TEST
