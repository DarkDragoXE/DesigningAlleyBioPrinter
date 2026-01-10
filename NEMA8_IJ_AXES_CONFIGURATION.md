# NEMA 8 Motor Configuration (I and J Axes)
**Date:** 2026-01-10
**Motor:** JKongMotor NEMA 8 JK20HST30-0604
**Application:** Independent printhead Z height adjustment
**Axes:** I (Motor 6) and J (Motor 7)

---

## Motor Specifications

**Source:** https://www.jkongmotor.com/nema-8-jk20hst30-0604-linear-stepper-motor.html

### Electrical Specifications
- **Model:** JK20HST30-0604
- **Type:** NEMA 8 External T-type Linear Stepper Motor
- **Step Angle:** 1.8° (200 steps per revolution)
- **Rated Current:** 0.6A (600mA) ← **Critical for TMC2209**
- **Resistance:** 6.5Ω ± 10% @ 20°C
- **Rated Torque:** 1.8 N·cm (2.55 oz·in)
- **Phase:** 2-phase bipolar
- **Leads:** 4 wires

### Mechanical Specifications
- **Size:** NEMA 8 (20mm × 20mm faceplate)
- **Body Length:** 30mm
- **Lead Screw:** External T-type linear (assumed 1mm pitch)
- **Lead:** 1mm per revolution (assumed - **VERIFY THIS**)

---

## Firmware Configuration

### Steps/mm Calculation

**Assumed Lead Screw:** 1mm pitch, 1mm lead (single-start thread)

**Formula:**
```
Steps/mm = (Motor steps/rev × Microstepping) / Lead (mm)
```

**Calculation:**
```
Steps/mm = (200 steps/rev × 16 microsteps) / 1mm lead
         = 3200 / 1
         = 3200 steps/mm
```

**Resolution:**
```
Travel per microstep = 1mm / 3200 = 0.3125 microns (312.5 nanometers)
```

**Verification:**
```
Travel per full step = 1mm / 200 = 0.005mm = 5 microns
```

---

## Configuration Changes

### Configuration.h (Line 1045)

**BEFORE:**
```cpp
#define DEFAULT_AXIS_STEPS_PER_UNIT   { 80, 80, 400, 400, 400, 3200, 500 } // I/J placeholder: 400 steps/mm
```

**AFTER:**
```cpp
#define DEFAULT_AXIS_STEPS_PER_UNIT   { 80, 80, 400, 3200, 3200, 3200, 500 }  // X, Y, Z, I (NEMA 8 Tr?×1mm: 3200), J (NEMA 8 Tr?×1mm: 3200), E0 (NEMA 11), E1
```

**Why:**
- Motor: 200 steps/rev × 16 microsteps = 3200 steps/rev
- Lead screw: 1mm per revolution (assumed)
- Steps/mm = 3200 / 1 = **3200 steps/mm**

**Impact:**
- ✅ Correct linear positioning (0.3125 micron resolution)
- ✅ Accurate printhead height adjustment
- ✅ Precise tool offset calibration

---

### Configuration_adv.h (Lines 2909, 2919)

**BEFORE:**
```cpp
#define I_CURRENT      800  // Placeholder
#define J_CURRENT      800  // Placeholder
```

**AFTER:**
```cpp
#define I_CURRENT      600  // BIOPRINTER: NEMA 8 JK20HST30-0604 rated current (0.6A)
#define J_CURRENT      600  // BIOPRINTER: NEMA 8 JK20HST30-0604 rated current (0.6A)
```

**Why:**
- Manufacturer rated current: 0.6A = 600mA
- Previous setting: 800mA = **33.3% overcurrent**
- Risk of overheating and motor damage

**Impact:**
- ✅ Motor runs at rated specifications
- ✅ Prevents overheating
- ✅ Long motor lifespan
- ✅ Reliable operation

---

## Hardware Assignment

| Axis | Motor Slot | BTT Octopus Connector | Purpose | Driver | Current |
|------|------------|----------------------|---------|--------|---------|
| **I** | **Motor 6** | **E2** | Printhead 0 (E0) Z height | TMC2209 | 600mA |
| **J** | **Motor 7** | **E3** | Printhead 1 (E1) Z height | TMC2209 | 600mA |

### Pin Assignments (from pins_BTT_OCTOPUS_V1_common.h)

**I Axis (Motor 6 / E2 Hardware):**
- **STEP:** PE2
- **DIR:** PE3
- **ENABLE:** PD4
- **CS (TMC2209):** PE1
- **UART TX/RX:** PE1
- **Endstop (MIN):** PG13 (E1DET connector)

**J Axis (Motor 7 / E3 Hardware):**
- **STEP:** PE6
- **DIR:** PA14
- **ENABLE:** PE0
- **CS (TMC2209):** PD3
- **UART TX/RX:** PD3
- **Endstop (MIN):** PG14 (E2DET connector)

---

## Motion Parameters

| Parameter | I Axis | J Axis | Notes |
|-----------|--------|--------|-------|
| **Steps/mm** | 3200 | 3200 | Calculated for 1mm lead screw |
| **Max Feedrate** | 5 mm/s | 5 mm/s | Conservative for printhead movement |
| **Max Acceleration** | 100 mm/s² | 100 mm/s² | Gentle for precise positioning |
| **Homing Feedrate** | 5 mm/s (300 mm/min) | 5 mm/s (300 mm/min) | Safe homing speed |
| **Homing Direction** | MIN (-1) | MIN (-1) | Home downward to endstops |
| **Travel Range** | 0-50mm | 0-50mm | Software limits (placeholder) |
| **Motor Current** | 600mA | 600mA | Rated current (0.6A) |
| **Microstepping** | 16 | 16 | TMC2209 default |

---

## G-Code Commands

### Homing (INDEPENDENT - NOT triggered by G28)

```gcode
G28          # Homes X, Y, Z only (I and J axes NOT included)
G28 U        # Home I axis only (displayed as U in interface)
G28 V        # Home J axis only (displayed as V in interface)
```

**Important:** I and J axes do **NOT** home with `G28` - you must explicitly home them with `G28 U` or `G28 V`.

### Movement

```gcode
# Move I axis (Printhead 0 Z height):
G1 U10 F300    # Move I axis to 10mm at 5mm/s

# Move J axis (Printhead 1 Z height):
G1 V5 F300     # Move J axis to 5mm at 5mm/s

# Combined movement:
G1 U15 V10 F300  # Move both printheads
```

### Configuration

```gcode
# Verify/set steps/mm:
M92 U3200      # Set I axis steps/mm
M92 V3200      # Set J axis steps/mm

# Verify/set motor current:
M906 I600      # Set I axis current (600mA)
M906 J600      # Set J axis current (600mA)

# Save to EEPROM:
M500           # Save all settings

# Display current settings:
M503           # Show all configuration
```

### Endstop Testing

```gcode
M119           # Report endstop states
               # Should show U_MIN and V_MIN status
```

---

## Force and Performance Calculations

### Linear Force Capability

**Formula:**
```
Force = (Torque × 2π) / Lead
```

**Calculation:**
```
F = (0.018 N·m × 6.283) / 0.001m
  = 113.09 N
  ≈ 11.5 kg force
```

**Bioprinting Requirement:** Typically 5-50N (this motor provides 113N ✓)

### Speed Capabilities

**Max Feedrate:** 5 mm/s (300 mm/min)
- Steps per second = 3200 steps/mm × 5 mm/s = **16,000 steps/s**
- Well within TMC2209 capabilities (~150,000 steps/s max)

**Typical Bioprinting Speeds:** 0.5-2 mm/s
- Motor is overspecified for speed requirements ✓

---

## Testing Procedures

### Test 1: Verify Steps/mm

```gcode
M302 P1          # Allow cold movement (no heater required)
M92 U3200        # Set I axis steps/mm
M92 V3200        # Set J axis steps/mm
M500             # Save to EEPROM
M503             # Display settings
```

**Expected Output:**
```
echo:  M92 X80.00 Y80.00 Z400.00 U3200.00 V3200.00 E3200.00 E500.00
```

---

### Test 2: Linear Movement Calibration

```gcode
M302 P1          # Allow cold movement
M83              # Relative positioning mode
G28 U            # Home I axis
G1 U10 F300      # Move I axis 10mm
```

**Measure:** Linear movement = 10mm ± 0.1mm with calipers or ruler

**Adjust if needed:**
- If movement is less than 10mm: steps/mm too high → decrease value
- If movement is more than 10mm: steps/mm too low → increase value

**Formula to adjust:**
```
New steps/mm = Current steps/mm × (10mm / Actual movement)
```

---

### Test 3: Motor Current Verification

```gcode
M906 I600        # Set I axis current
M906 J600        # Set J axis current
M500             # Save
M122             # TMC2209 debug info
```

**Check:** TMC2209 reports run current = 600mA for I and J axes

**Listen/Feel:**
- Motor should run cool (not hot to touch after 10 minutes)
- No grinding or skipping sounds
- Smooth movement

---

### Test 4: Endstop Verification

```gcode
M119             # Report endstop states
```

**Expected Output:**
```
Reporting endstop status
x_min: open
y_min: open
z_min: open
u_min: TRIGGERED/open  (depending on endstop state)
v_min: TRIGGERED/open  (depending on endstop state)
```

**Test:** Manually trigger I and J endstops, verify M119 shows "TRIGGERED"

---

### Test 5: Homing Test

```gcode
G28 U            # Home I axis
M114             # Report current position
```

**Expected:** I axis position should be 0.00 after homing

**Repeat for J axis:**
```gcode
G28 V            # Home J axis
M114             # Report current position
```

---

## CRITICAL: Lead Screw Verification Required

⚠️ **IMPORTANT:** The configuration assumes **1mm pitch/lead** screw, but this needs verification from the motor datasheet or physical measurement.

### How to Verify Lead Screw Pitch:

1. **Physical Measurement:**
   - Remove motor from system
   - Mark starting position of lead screw nut
   - Rotate motor shaft exactly 1 full revolution (360°)
   - Measure linear travel distance = **Lead** value

2. **Datasheet Check:**
   - Look for "Travel per revolution" or "Lead" specification
   - Common NEMA 8 values: 1mm, 2mm, or 2.5mm

3. **If Lead is NOT 1mm:**
   - Update steps/mm calculation:
     ```
     Steps/mm = 3200 / Lead (mm)
     ```
   - Examples:
     - 2mm lead → 3200 / 2 = **1600 steps/mm**
     - 2.5mm lead → 3200 / 2.5 = **1280 steps/mm**

---

## Troubleshooting

### Motor Overheating
- **Cause:** Current too high
- **Fix:** Verify I_CURRENT and J_CURRENT = 600mA
- **Check:** Motor should be warm but touchable after 10 min operation

### Incorrect Travel Distance
- **Cause:** Wrong steps/mm (lead screw pitch mismatch)
- **Fix:** Measure actual lead screw pitch, recalculate steps/mm
- **Test:** Use Test 2 calibration procedure

### Skipping Steps
- **Cause:** Current too low, speed too high, or mechanical binding
- **Fix:**
  - Check motor current = 600mA
  - Reduce feedrate if needed
  - Check for mechanical obstructions

### Endstops Not Triggering
- **Cause:** Wrong pin assignment or bad wiring
- **Fix:**
  - Verify E1DET (PG13) and E2DET (PG14) connections
  - Test with M119 command
  - Check endstop wiring polarity

### Homing Fails
- **Cause:** Endstop not detected or wrong homing direction
- **Fix:**
  - Verify M119 shows endstop status changes
  - Check I_HOME_DIR and J_HOME_DIR = -1 (Configuration.h)
  - Verify endstop wiring

---

## Summary

### Current Status:
✅ Steps/mm configured: **3200** (assuming 1mm lead)
✅ Motor current configured: **600mA** (rated)
✅ Pin assignments: Motor 6 (I), Motor 7 (J)
✅ Endstops assigned: E1DET (I), E2DET (J)
✅ TMC2209 UART configured
⏳ **Lead screw pitch verification PENDING**
⏳ Physical installation and testing PENDING

### Next Steps:
1. **VERIFY lead screw pitch** from motor or datasheet
2. **Install motors** in Motor 6 and Motor 7 slots
3. **Connect endstops** to E1DET and E2DET
4. **Flash firmware** (firmware.bin)
5. **Run test procedures** (Tests 1-5 above)
6. **Calibrate** steps/mm if needed
7. **Test printhead positioning** in actual print scenarios

---

**Configuration Complete:** 2026-01-10
**Build Status:** ✅ SUCCESS
**Firmware:** Ready for flashing
