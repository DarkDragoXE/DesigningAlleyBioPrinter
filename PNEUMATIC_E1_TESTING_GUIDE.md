# Pneumatic E1 Control - Testing Guide
**Created:** 2025-12-22
**Hardware:** BTT Octopus V1.1 + Pneumatic Dispensing System
**Pin:** PC3 (E1_ENABLE_PIN)

---

## Overview

E1 has been converted from stepper motor control to pneumatic valve control. The E1_ENABLE_PIN (PC3) now acts as a digital solenoid/valve signal:

- **PC3 = HIGH:** Valve OPEN → Material dispenses
- **PC3 = LOW:** Valve CLOSED → No dispensing

## Hardware Setup

### Required Connections
```
BTT Octopus E1 Motor Connector (Motor 4):
├── PC3 (ENABLE pin) → Pneumatic control board SIGNAL INPUT
├── GND              → Pneumatic control board GND (common ground)
└── VCC (optional)   → Not used for signal control
```

### Pneumatic Control Board
- **Input:** Digital signal from PC3 (3.3V logic)
- **Output:** Controls solenoid valve or pneumatic regulator
- **Pressure Source:** Connected to your external pneumatic system

### Safety Checks
- ✅ Verify pneumatic pressure is set correctly (typically 10-30 PSI for bioprinting)
- ✅ Test valve manually before connecting to firmware
- ✅ Ensure emergency shutoff is accessible
- ✅ Check for air leaks in pneumatic lines

---

## Firmware Configuration

### Enabled in Configuration_adv.h (Line 710)
```cpp
#define PNEUMATIC_EXTRUDER_E1
//#define DEBUG_PNEUMATIC_EXTRUDER  // Uncomment for detailed serial output
```

### Build & Flash
```bash
cd c:\BIOPRINTER\BTTOctopusCursor\OctopusMarlin-bugfix-test
pio run -e STM32F446ZE_btt
# Flash with ST-Link or your preferred method
```

---

## G-code Testing

### Test 1: Tool Selection
**Objective:** Verify tool switching between E0 (stepper) and E1 (pneumatic)

```gcode
M302 P1          ; Enable cold extrusion (if not auto-enabled)
T0               ; Select E0 (syringe stepper motor)
M114             ; Report position
T1               ; Select E1 (pneumatic dispenser)
M114             ; Report position
T0               ; Back to E0
```

**Expected Behavior:**
- No errors during tool changes
- Serial output confirms tool selection
- PC3 remains LOW (valve closed) during tool change

---

### Test 2: Manual Valve Control (Quick Test)
**Objective:** Verify PC3 pin toggles correctly

```gcode
T1               ; Select E1
M42 P{PC3} S255  ; Force PC3 HIGH (valve open) - may need pin number lookup
M400             ; Wait for moves to finish
G4 P1000         ; Dwell 1 second
M42 P{PC3} S0    ; Force PC3 LOW (valve closed)
```

**Expected Behavior:**
- You should hear/see the pneumatic valve actuate
- Material should dispense when valve opens (if pressure is connected)
- Valve should close cleanly

**Note:** Replace `{PC3}` with the actual pin number (you may need to check `pins_BTT_OCTOPUS_V1_common.h` for the numeric value)

---

### Test 3: Simple Extrusion Move
**Objective:** Test automatic valve control during E-moves

```gcode
T1               ; Select pneumatic extruder
G92 E0           ; Set extruder position to 0
G1 E5 F180       ; Extrude 5mm at 180mm/min (3mm/s)
G4 P500          ; Dwell 0.5 seconds
G1 E10 F180      ; Extrude to 10mm position
G92 E0           ; Reset position
```

**Expected Behavior:**
- **During G1 E5:** PC3 goes HIGH for ~1.67 seconds (5mm / 3mm/s)
- **During G1 E10:** PC3 goes HIGH for ~1.67 seconds (5mm / 3mm/s)
- **Between moves:** PC3 returns to LOW
- Material dispenses during each extrusion command

**Timing Calculation:**
```
Distance: 5mm
Speed: 180mm/min = 3mm/s
Duration: 5mm / 3mm/s = 1.67 seconds
→ PC3 should be HIGH for 1.67 seconds
```

---

### Test 4: Variable Speed Extrusion
**Objective:** Verify valve timing matches extrusion speed

```gcode
T1               ; Select pneumatic
G92 E0

; Slow extrusion (10mm at 60mm/min = 1mm/s → 10 seconds)
G1 E10 F60
G4 P1000

; Medium speed (10mm at 180mm/min = 3mm/s → 3.3 seconds)
G1 E20 F180
G4 P1000

; Fast extrusion (10mm at 300mm/min = 5mm/s → 2 seconds)
G1 E30 F300

G92 E0
```

**Expected Behavior:**
- Slow: PC3 HIGH for 10 seconds
- Medium: PC3 HIGH for 3.3 seconds
- Fast: PC3 HIGH for 2 seconds

**Observation:** Longer extrusion times = more material dispensed (pressure-dependent)

---

### Test 5: Retraction Behavior
**Objective:** Verify valve closes during retraction

```gcode
T1               ; Select pneumatic
G92 E0

G1 E10 F180      ; Extrude 10mm (valve should open)
G1 E5 F180       ; Retract 5mm (valve should close - moving backward)
G1 E10 F180      ; Extrude again (valve should open)

G92 E0
```

**Expected Behavior:**
- **G1 E10:** PC3 = HIGH (forward extrusion)
- **G1 E5:** PC3 = LOW (retraction, negative E movement)
- **G1 E10 (second):** PC3 = HIGH (forward extrusion again)

**Important:** Pneumatic systems don't retract! The valve just closes. No material should dispense during retraction moves.

---

### Test 6: Combined X/Y Movement with Extrusion
**Objective:** Test valve control during actual printing moves

```gcode
T1               ; Select pneumatic
G92 E0
G28 X Y          ; Home X and Y
G0 X50 Y50 F3000 ; Move to start position (no extrusion, valve closed)

; Draw a 20mm line while extruding
G1 X70 Y50 E5 F600   ; 20mm move at 600mm/min (10mm/s), extrude 5mm
G4 P500

; Draw another line
G1 X70 Y70 E10 F600  ; 20mm move, extrude to E=10

G92 E0
G0 X0 Y0         ; Return to origin (no extrusion)
```

**Expected Behavior:**
- **Travel move (G0):** PC3 = LOW (no extrusion)
- **Print move (G1 with E):** PC3 = HIGH during the entire move
- **Duration:** Each line takes 2 seconds (20mm / 10mm/s), so PC3 is HIGH for 2 seconds per line

---

### Test 7: Multi-Material Switching
**Objective:** Verify smooth transitions between E0 (stepper) and E1 (pneumatic)

```gcode
; Print with E0 (syringe stepper)
T0
G92 E0
G1 E10 F300      ; Extrude with stepper motor
G4 P1000

; Switch to E1 (pneumatic)
T1
G92 E0
G1 E10 F300      ; Extrude with pneumatic (PC3 goes HIGH)
G4 P1000

; Switch back to E0
T0
G92 E0
G1 E10 F300      ; Back to stepper motor
```

**Expected Behavior:**
- T0 extrusion: E0 motor rotates, PC3 = LOW
- T1 extrusion: E0 motor idle, PC3 = HIGH
- No interference between extruders

---

## Debugging

### Enable Debug Output
In `Configuration_adv.h` (line 711):
```cpp
#define DEBUG_PNEUMATIC_EXTRUDER
```

Rebuild and flash. Serial output will show:
```
Pneumatic E1: Tool change to T1 (PNEUMATIC ACTIVE)
Pneumatic E1: Valve OPEN (extruding)
Pneumatic E1: Valve CLOSED (duration: 1670ms)
```

### Common Issues

**Issue:** Valve doesn't open during extrusion
- **Check:** Is T1 selected? (`M114` or `T1`)
- **Check:** Is there E movement? (E value must increase)
- **Check:** Wiring to pneumatic board correct?
- **Test:** Manual M42 command to toggle PC3

**Issue:** Valve stays open after extrusion
- **Check:** Firmware compiled with PNEUMATIC_EXTRUDER_E1?
- **Check:** Proper end-of-move detection
- **Test:** Send `G4 P1` after extrusion to force move completion

**Issue:** Intermittent valve operation
- **Check:** Grounding between Octopus and pneumatic board
- **Check:** Signal voltage (should be 3.3V when HIGH)
- **Check:** Pneumatic board input impedance

---

## Pin Verification

### Check PC3 State Manually
```gcode
M43 P{PC3}       ; Report PC3 pin state (replace {PC3} with pin number)
```

### PC3 Physical Location on BTT Octopus V1.1
- **Connector:** Motor 4 (E1) connector
- **Position:** ENABLE pin (usually middle pin in Step/Dir/Enable group)
- **Voltage:** 3.3V when HIGH, 0V when LOW

---

## Safety & Best Practices

### Pre-Print Checklist
- [ ] Verify pneumatic pressure (10-30 PSI typical)
- [ ] Test valve manually before starting print
- [ ] Ensure T1 is selected for pneumatic dispensing
- [ ] Have emergency air shutoff accessible
- [ ] Monitor first few layers for proper material flow

### During Printing
- Monitor material flow rate (should match extrusion speed × pressure)
- Adjust pneumatic pressure if flow is too fast/slow
- Watch for air leaks
- Check that valve closes between moves (no drooling)

### Post-Print
- Select T0 to disable pneumatic (valve closes)
- Release pneumatic pressure
- Clean dispensing tip/nozzle

---

## Calibration Notes

### Pressure vs. Flow Rate
The firmware controls valve **timing** (how long it's open), but **flow rate** is determined by:
- Pneumatic pressure setting
- Bioink viscosity
- Nozzle diameter
- Valve opening size

**Calibration Procedure:**
1. Set a fixed extrusion speed (e.g., `G1 E10 F180`)
2. Adjust pneumatic pressure until desired flow rate achieved
3. Record pressure setting for that bioink
4. Repeat for different bioinks/nozzles

### Example Calibration
```
Bioink: Alginate 3%
Nozzle: 22G (0.41mm ID)
Extrusion command: G1 E10 F180 (10mm at 3mm/s)
Valve open time: 3.33 seconds

Pressure test results:
- 10 PSI: Too slow, ~2mm actual dispense
- 15 PSI: Good, ~10mm actual dispense ✅
- 20 PSI: Too fast, ~15mm actual dispense

→ Use 15 PSI for this bioink
```

---

## Integration with Slic3r/PrusaSlicer/Cura

### Setup T1 as Pneumatic Extruder
1. In slicer, add a second extruder
2. Configure T1 extruder settings:
   - **Nozzle diameter:** Match your pneumatic nozzle (e.g., 0.41mm for 22G)
   - **Extrusion multiplier:** Start at 1.0, adjust based on calibration
   - **Retraction:** Set to 0mm (pneumatic doesn't retract!)
   - **Max speed:** Match your calibrated extrusion speed (e.g., 5mm/s)

3. Assign materials to extruders:
   - E0: Standard bioink (syringe)
   - E1: Pneumatic bioink

4. Slice and print!

---

## Troubleshooting Matrix

| Symptom | Possible Cause | Solution |
|---------|----------------|----------|
| No dispensing | Valve not opening | Check PC3 wiring, verify HIGH signal |
| Continuous dispensing | Valve stuck open | Check pneumatic board, verify PC3 goes LOW |
| Inconsistent flow | Pressure fluctuations | Add pressure regulator, check air supply |
| Drooling between moves | Valve closing too slow | Adjust valve response time, check for backpressure |
| Too much material | Pressure too high | Reduce pneumatic pressure setting |
| Too little material | Pressure too low | Increase pneumatic pressure setting |
| E1 acts like stepper | PNEUMATIC_EXTRUDER_E1 not defined | Recompile with feature enabled |

---

## Next Steps

1. **Hardware Test:** Verify PC3 toggles valve before loading bioink
2. **Calibration:** Find optimal pressure for your bioink
3. **Simple Shapes:** Print test patterns (lines, squares, circles)
4. **Multi-Material:** Test switching between E0 and E1
5. **Production:** Integrate into your bioprinting workflow

---

## Support & Documentation

- **Firmware Source:** `Marlin/src/feature/pneumatic_extruder.cpp`
- **Configuration:** `Configuration_adv.h` line 710
- **Pin Definition:** `pins_BTT_OCTOPUS_V1_common.h` line 236
- **Stepper Integration:** `stepper.cpp` lines 2348-2355
- **Project Memory:** See `CLAUDE.md` for full project context

**Testing Complete! Ready for bioprinting with pneumatic E1 control.**
