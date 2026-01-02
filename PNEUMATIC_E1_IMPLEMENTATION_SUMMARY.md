# Pneumatic E1 Implementation - Summary
**Date:** 2025-12-22
**Feature:** Pneumatic Dispenser Control for E1
**Pin:** PC3 (E1_ENABLE_PIN on BTT Octopus V1.1)

---

## What Was Implemented

Your E1 extruder has been converted from stepper motor control to pneumatic valve control. Now:

- **T0:** Controls E0 as a stepper motor (syringe-based bioink)
- **T1:** Controls E1 as a pneumatic dispenser via PC3 pin

### How It Works

When you select T1 and send extrusion commands:
```gcode
T1              ; Select pneumatic extruder
G1 E10 F180     ; Extrude 10mm at 180mm/min
```

**What happens:**
1. Firmware calculates extrusion duration: 10mm / 3mm/s = 3.33 seconds
2. PC3 goes HIGH for exactly 3.33 seconds
3. Your pneumatic valve opens → material dispenses
4. After 3.33 seconds, PC3 goes LOW → valve closes
5. No material flows between moves

---

## Files Modified/Created

### New Files Created
1. **[Marlin/src/feature/pneumatic_extruder.h](Marlin/src/feature/pneumatic_extruder.h)**
   - Header file with pneumatic control class definition
   - Defines valve control functions

2. **[Marlin/src/feature/pneumatic_extruder.cpp](Marlin/src/feature/pneumatic_extruder.cpp)**
   - Implementation of pneumatic control logic
   - Handles valve open/close timing

3. **[PNEUMATIC_E1_TESTING_GUIDE.md](PNEUMATIC_E1_TESTING_GUIDE.md)**
   - Comprehensive testing procedures
   - G-code examples for all scenarios
   - Troubleshooting guide
   - Calibration instructions

4. **PNEUMATIC_E1_IMPLEMENTATION_SUMMARY.md** (this file)
   - Quick reference summary

### Modified Files

1. **[Marlin/Configuration_adv.h](Marlin/Configuration_adv.h#L710)**
   - Added `#define PNEUMATIC_EXTRUDER_E1` (line 710)
   - Added optional `#define DEBUG_PNEUMATIC_EXTRUDER` (line 711)
   - Includes detailed usage comments

2. **[Marlin/src/module/stepper/indirection.h](Marlin/src/module/stepper/indirection.h#L340-L375)**
   - Modified E1 pin control macros (lines 340-375)
   - E1_STEP_WRITE → NOOP (no stepping for pneumatic)
   - E1_DIR_WRITE → NOOP (no direction for pneumatic)
   - E1_ENABLE_WRITE → Controls PC3 valve signal

3. **[Marlin/src/module/stepper.cpp](Marlin/src/module/stepper.cpp#L2348-L2355)**
   - Added pneumatic start hook (lines 2348-2355)
   - Turns PC3 HIGH when E1 extrusion begins
   - Checks: `if (stepper_extruder == 1 && current_block->steps.e > 0)`

4. **[Marlin/src/module/stepper.h](Marlin/src/module/stepper.h#L524-L531)**
   - Modified `discard_current_block()` function (lines 524-531)
   - Turns PC3 LOW when extrusion ends
   - Ensures valve closes cleanly after each move

5. **[CLAUDE.md](CLAUDE.md#L189-L225)**
   - Updated project memory with pneumatic feature documentation
   - Added to "Work Completed" section

---

## Configuration

### Enabled by Default
The feature is **enabled by default** in your firmware:

```cpp
// Configuration_adv.h line 710
#define PNEUMATIC_EXTRUDER_E1
```

### Debug Mode (Enabled by Default)
Detailed serial output is **enabled by default** for testing:

```cpp
// Configuration_adv.h line 711
#define DEBUG_PNEUMATIC_EXTRUDER  // Detailed timing and duration info
```

This prints detailed timing information:
```
Pneumatic E1: Valve OPEN - E steps: 5000, Total step_events: 5000, Rate: 500 steps/s, Expected duration: 10s
Pneumatic E1: Valve CLOSED - Actual duration: 10000ms
```

This allows you to verify that the valve timing **exactly matches** how long a stepper motor would take for the same extrusion command.

---

## Build & Flash Instructions

### 1. Build Firmware
```bash
cd c:\BIOPRINTER\BTTOctopusCursor\OctopusMarlin-bugfix-test
pio run -e STM32F446ZE_btt
```

Expected output:
```
Processing STM32F446ZE_btt (platform: ststm32; board: STM32F446ZE_btt; framework: arduino)
...
Building .pio\build\STM32F446ZE_btt\firmware.bin
SUCCESS
```

### 2. Flash to Board
Use ST-Link or your preferred method:
```bash
# Locate firmware file:
.pio\build\STM32F446ZE_btt\firmware.bin

# Flash with ST-Link (example):
st-flash write firmware.bin 0x8000000
```

### 3. Verify
After flashing:
1. Connect via serial (115200 baud)
2. Send `M115` to check firmware info
3. Look for successful boot messages

---

## Hardware Connection

### Wiring Diagram
```
BTT Octopus V1.1                    Pneumatic Control Board
┌──────────────┐                    ┌────────────────────┐
│ Motor 4 (E1) │                    │                    │
│ ┌──────────┐ │                    │  Signal Inputs:    │
│ │ STEP     │ │ (not used)         │                    │
│ │ DIR      │ │ (not used)         │  ┌──────────────┐  │
│ │ ENABLE ──┼─┼──── PC3 ──────────┼──┤ VALVE SIGNAL │  │
│ │ GND    ──┼─┼──── GND ──────────┼──┤ GND          │  │
│ └──────────┘ │                    │  └──────────────┘  │
└──────────────┘                    │                    │
                                    │  Outputs:          │
                                    │  → Solenoid Valve  │
                                    │  → Pneumatic       │
                                    │     Pressure Reg   │
                                    └────────────────────┘
                                             │
                                             ↓
                                    [ Air Pressure Source ]
                                         (10-30 PSI)
```

### Pin Specifications
- **PC3 Logic:** 3.3V (HIGH) / 0V (LOW)
- **Current Capacity:** 10mA max (use optocoupler if higher current needed)
- **Frequency:** Variable (depends on G-code extrusion timing)

---

## Quick Test Sequence

After flashing firmware, run this quick test:

```gcode
; === PNEUMATIC E1 QUICK TEST ===

; 1. Select pneumatic extruder
T1

; 2. Test short extrusion (2 seconds)
G92 E0
G1 E6 F180
G4 P1000

; 3. Test medium extrusion (5 seconds)
G1 E21 F180
G4 P1000

; 4. Test with movement
G28 X Y
G0 X50 Y50
G1 X70 Y70 E36 F600

; 5. Return to E0
T0
```

**Expected:**
- PC3 goes HIGH for 2s, 5s during extrusions
- Pneumatic valve actuates (you should hear it)
- Material dispenses when valve is open
- Valve closes between moves

---

## Functional Flow

### Extrusion Move Lifecycle

```
User sends: G1 E10 F180
     │
     ↓
[Planner calculates move]
     │
     ↓
[Stepper receives block]
     │
     ↓
Is stepper_extruder == 1?  ──── NO ───→ [E0 motor steps normally]
     │
     YES
     ↓
Is current_block->steps.e > 0?  ──── NO ───→ [PC3 stays LOW]
     │
     YES
     ↓
**PC3 = HIGH** (Valve OPEN)
     │
[Stepper executes move timing]
     │  (10mm / 3mm/s = 3.33 seconds)
     │
     ↓
[Block complete]
     │
     ↓
discard_current_block()
     │
     ↓
**PC3 = LOW** (Valve CLOSED)
```

---

## Key Differences from Standard Marlin

### Normal E1 (Stepper Motor)
```cpp
E1_STEP_WRITE(HIGH/LOW)  // Sends step pulses
E1_DIR_WRITE(HIGH/LOW)   // Sets rotation direction
E1_ENABLE_WRITE(LOW)     // Enables motor driver
```

### Your E1 (Pneumatic)
```cpp
E1_STEP_WRITE(STATE)  → NOOP  // No stepping
E1_DIR_WRITE(STATE)   → NOOP  // No direction
E1_ENABLE_WRITE(HIGH) → PC3 = HIGH  // Opens valve
E1_ENABLE_WRITE(LOW)  → PC3 = LOW   // Closes valve
```

**Result:** PC3 acts as pure digital valve control signal

---

## Integration with Slicers

### PrusaSlicer/Slic3r
1. Printer Settings → Extruders → Add extruder
2. Configure T1:
   - Nozzle diameter: Match pneumatic nozzle (e.g., 0.41mm for 22G)
   - Retraction length: 0mm (no retraction for pneumatic!)
   - Max volumetric speed: Based on pressure calibration

3. Filament Settings → Create pneumatic material profile
4. Assign materials to tools

### Cura
1. Settings → Printer → Machine Settings
2. Add Extruder 2
3. Configure nozzle size and material flow
4. Disable retraction for Extruder 2

---

## Calibration Workflow

1. **Set baseline pressure:** Start at 15 PSI
2. **Test extrusion:** Send `G1 E10 F180` (10mm at 3mm/s)
3. **Measure actual output:** Weigh or measure dispensed volume
4. **Adjust:**
   - Too much → Reduce pressure
   - Too little → Increase pressure
5. **Record settings** for each bioink type

---

## Troubleshooting Quick Reference

| Problem | Check | Fix |
|---------|-------|-----|
| No dispensing | T1 selected? | Send `T1` |
| | PC3 wiring | Verify connection |
| | Pneumatic pressure | Check air supply |
| Valve won't close | Firmware compiled? | Rebuild with feature |
| | Hardware issue | Test valve manually |
| Wrong duration | G-code speed | Check F parameter |
| | Firmware timing | Enable DEBUG mode |

---

## Safety Notes

⚠️ **IMPORTANT**
- Always test with water/air before using bioink
- Monitor first extrusion closely
- Have emergency air shutoff accessible
- Never exceed recommended pressure for bioink
- PC3 is 3.3V logic - use level shifter if needed

---

## Success Criteria

✅ Firmware builds without errors
✅ PC3 toggles HIGH/LOW correctly
✅ Valve actuates on extrusion commands
✅ Timing matches G-code parameters
✅ No dispensing during travel moves
✅ Clean switching between T0 and T1
✅ Material flow is controllable via pressure

---

## Next Steps

1. **Hardware Test:** Flash firmware, verify PC3 with multimeter
2. **Pneumatic Test:** Connect valve, test with air only
3. **Flow Calibration:** Test with water, then bioink
4. **Print Test:** Simple test patterns (lines, squares)
5. **Multi-Material:** Test E0 ↔ E1 switching
6. **Production:** Integrate into bioprinting workflow

---

## Support & References

- **Testing Guide:** [PNEUMATIC_E1_TESTING_GUIDE.md](PNEUMATIC_E1_TESTING_GUIDE.md)
- **Project Memory:** [CLAUDE.md](CLAUDE.md#L189-L225)
- **Pin Definition:** [pins_BTT_OCTOPUS_V1_common.h:236](Marlin/src/pins/stm32f4/pins_BTT_OCTOPUS_V1_common.h#L236)
- **Configuration:** [Configuration_adv.h:710](Marlin/Configuration_adv.h#L710)

**Implementation Complete! Ready for testing.**
