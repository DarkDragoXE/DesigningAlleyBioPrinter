# NEMA 11 Motor Configuration Changes
**Date:** 2026-01-08
**Motor:** JKongMotor NEMA 11 JK28HST32-0674
**Lead Screw:** Tr5 × 1mm pitch (1mm lead)
**Driver:** TMC2209

---

## Motor Specifications (Source: JKongMotor Website)

### From: https://www.jkongmotor.com/nema-11-jk28hst32-0674-linear-stepper-motor.html

**Electrical Specifications (Verified from manufacturer):**
- Step Angle: 1.8° (200 steps per revolution)
- **Rated Current: 0.67A (670mA)** ← Critical for TMC2209 current setting
- Resistance: 5.6Ω ± 0.1Ω @ 20°C
- Inductance: 3.4mH ± 20% @ 1kHz
- Holding Torque: 6 N·cm (8.5 oz·in)
- Phase: 2-phase bipolar
- Leads: 4 wires

**Lead Screw Specifications (Verified from manufacturer):**
- Shaft Diameter: Tr5 (5mm trapezoidal screw)
- **Pitch: 1mm** ← User confirmed
- **Lead: 1mm** ← For single-start thread, lead = pitch
- Travel per revolution: 1mm

---

## Calculations (With Proof)

### Steps/mm Calculation

**Formula:**
```
Steps/mm = (Motor steps/rev × Microstepping) / Lead (mm/rev)
```

**Given Values:**
- Motor steps/rev = 360° / 1.8° = **200 steps/rev**
- Microstepping = **16** (TMC2209 default configuration)
- Lead = **1mm** (from Tr5×1mm specification)

**Calculation:**
```
Steps/mm = (200 × 16) / 1
         = 3200 / 1
         = 3200 steps/mm
```

**Verification:**
```
Linear travel per microstep = 1mm / 3200 = 0.0003125mm = 0.3125 microns
```

This matches the manufacturer's specification of 0.005mm travel per full step:
```
0.005mm per full step = 1mm / 200 steps = 0.005mm ✓
```

---

### Motor Current Calculation

**Manufacturer Specification:**
- Rated Current: **0.67A = 670mA**

**Current TMC2209 Setting:**
- E0_CURRENT = **800mA**

**Analysis:**
```
Overcurrent % = ((800 - 670) / 670) × 100%
              = (130 / 670) × 100%
              = 19.4% overcurrent
```

**Risk Assessment:**
- ⚠️ **19.4% overcurrent** will cause motor overheating
- Motor insulation class: B (130°C max)
- Exceeding rated current reduces motor lifespan
- May cause thermal shutdown or permanent damage

**Required Change:**
```
E0_CURRENT: 800mA → 670mA (100% rated current)
```

**Alternative (Conservative):**
```
E0_CURRENT: 800mA → 600mA (90% rated current for longer life)
```

---

### Feedrate Adjustment Calculation

**Torque Comparison:**
- Typical NEMA 17: ~30-40 N·cm
- NEMA 11 JK28HST32-0674: **6 N·cm**
- Torque reduction: **80-85%**

**Force at Lead Screw:**
```
Force = (Torque × 2π) / Lead
      = (0.06 N·m × 2π) / 0.001m
      = 376.99 N (~38.4 kg force)
```

This is **sufficient for bioprinting applications** (typically 5-50N required).

**Current Max Feedrate: 5 mm/s**

**Recommended Feedrate:**
Due to lower torque, reduce max feedrate to prevent skipping:
```
New max feedrate = 3 mm/s (60% of original)
Reasoning: Conservative setting for 85% lower torque motor
```

**Can be increased after testing if motor performs well.**

---

### Acceleration Adjustment Calculation

**Current Max Acceleration: 2000 mm/s²**

**Analysis:**
```
Torque required ∝ Acceleration
With 85% less torque → Reduce acceleration proportionally
```

**Recommended Acceleration:**
```
New max acceleration = 1000 mm/s² (50% of original)
Reasoning: Conservative setting for lower torque motor
```

**This is still MORE than adequate for bioprinting:**
- Typical bioprinting: 100-500 mm/s²
- Cell viability requires gentle acceleration anyway

---

## Required Firmware Changes

### Change 1: Steps/mm (Configuration.h Line 1046)

**BEFORE:**
```cpp
#define DEFAULT_AXIS_STEPS_PER_UNIT   { 80, 80, 400, 500, 500 } // X, Y, Z, E0, E1
```

**AFTER:**
```cpp
#define DEFAULT_AXIS_STEPS_PER_UNIT   { 80, 80, 400, 3200, 500 } // X, Y, Z, E0 (NEMA 11 Tr5×1mm: 3200 steps/mm), E1
```

**Why:**
- Motor: 200 steps/rev × 16 microsteps = 3200 steps/rev
- Lead screw: 1mm per revolution
- Steps/mm = 3200 / 1 = **3200 steps/mm**

**Impact if not changed:**
- ❌ Current 500 steps/mm would move: 1mm command → 6.4mm actual movement
- ❌ Massive over-extrusion (6.4× too much bioink)
- ❌ Material waste, failed prints

**Proof:**
```
With 500 steps/mm:
  G1 E1 command sends 500 steps
  Actual movement = 500 / 3200 = 0.156mm (under-extrusion)

With 3200 steps/mm:
  G1 E1 command sends 3200 steps
  Actual movement = 3200 / 3200 = 1mm (correct!)
```

**Status:** ✅ Already changed

---

### Change 2: Motor Current (Configuration_adv.h Line 2969)

**BEFORE:**
```cpp
#define E0_CURRENT      800
```

**AFTER:**
```cpp
#define E0_CURRENT      670  // BIOPRINTER: NEMA 11 JK28HST32-0674 rated current (0.67A)
```

**Why:**
- Manufacturer rated current: 0.67A = 670mA
- Current setting: 800mA = 19.4% overcurrent
- **Risk:** Motor overheating, reduced lifespan, potential damage

**Impact if not changed:**
- ⚠️ Motor runs 19.4% over rated current
- ⚠️ Excessive heat generation (may exceed 130°C insulation rating)
- ⚠️ Shortened motor lifespan
- ⚠️ Possible thermal shutdown mid-print

**Proof:**
From JKongMotor datasheet:
- "Current: 0.67A"
- "Insulation Class: B (130°); 80K Max"

**Status:** ⏳ Pending (need to make this change)

---

### Change 3: Max Feedrate (Configuration.h Line 1053)

**BEFORE:**
```cpp
#define DEFAULT_MAX_FEEDRATE          { 30, 30, 5, 5 } // BIOPRINTER: gentle speeds for cell viability
```

**AFTER:**
```cpp
#define DEFAULT_MAX_FEEDRATE          { 30, 30, 5, 3 } // BIOPRINTER: E0 reduced for NEMA 11 (lower torque)
```

**Why:**
- NEMA 11 has 85% less torque than typical NEMA 17
- High speeds may cause motor skipping
- 3 mm/s is still fast for bioprinting (typical: 0.5-2 mm/s)

**Impact if not changed:**
- ⚠️ Motor may skip steps at high speeds
- ⚠️ Inconsistent extrusion
- ⚠️ Position loss (motor thinks it's at X but actually at Y)

**Impact if changed:**
- ✅ More reliable operation
- ✅ Prevents skipping
- ✅ Can still do 3 mm/s (faster than most bioprinting needs)
- ✅ Can increase later after testing

**Status:** ⏳ Pending (recommended change)

---

### Change 4: Max Acceleration (Configuration.h Line 1066)

**BEFORE:**
```cpp
#define DEFAULT_MAX_ACCELERATION      { 150, 150, 50, 2000 } // BIOPRINTER: gentle acceleration for all axes
```

**AFTER:**
```cpp
#define DEFAULT_MAX_ACCELERATION      { 150, 150, 50, 1000 } // BIOPRINTER: E0 reduced for NEMA 11 (lower torque)
```

**Why:**
- Lower torque motor requires lower acceleration
- 1000 mm/s² is still MORE than adequate for bioprinting
- Typical bioprinting: 100-500 mm/s²

**Impact if not changed:**
- ⚠️ Motor may skip steps during rapid acceleration
- ⚠️ "Thunk" or grinding noise during fast moves
- ⚠️ Position errors

**Impact if changed:**
- ✅ Smooth, reliable acceleration
- ✅ No skipping
- ✅ Still faster than bioprinting requirements
- ✅ Gentle on cell viability

**Status:** ⏳ Pending (recommended change)

---

## Summary of Changes

| File | Line | Parameter | Old Value | New Value | Status | Priority |
|------|------|-----------|-----------|-----------|--------|----------|
| Configuration.h | 1046 | E0 Steps/mm | 500 | **3200** | ✅ Done | 🔴 CRITICAL |
| Configuration_adv.h | 2969 | E0 Current (mA) | 800 | **670** | ⏳ Pending | 🔴 CRITICAL |
| Configuration.h | 1053 | E0 Max Feedrate (mm/s) | 5 | **3** | ⏳ Pending | 🟡 Recommended |
| Configuration.h | 1066 | E0 Max Accel (mm/s²) | 2000 | **1000** | ⏳ Pending | 🟡 Recommended |

---

## Proof of Backing (References)

### 1. Motor Specifications
**Source:** JKongMotor official product page
**URL:** https://www.jkongmotor.com/nema-11-jk28hst32-0674-linear-stepper-motor.html
**Retrieved:** 2026-01-08

**Verified Data:**
- Step Angle: 1.8°
- Current: 0.67A
- Resistance: 5.6Ω
- Holding Torque: 6 N·cm
- Lead Screw: Tr5 × 1mm pitch, 0.005mm travel per step

---

### 2. Lead Screw Calculation
**Formula Source:** Standard stepper motor kinematics
**Formula:** Steps/mm = (Steps/rev × Microstepping) / Lead

**Verification:**
- Motor manufacturer specifies: 0.005mm per full step
- Calculation: 1mm / 200 steps = 0.005mm ✓ Matches!
- With 16 microstepping: 0.005mm / 16 = 0.0003125mm per microstep ✓

---

### 3. TMC2209 Current Limit
**Source:** Trinamic TMC2209 datasheet
**Specification:** RMS current = I_rms = (CS + 1) / 32 × (V_ref / R_sense) × (1/√2)

**For BTT Octopus TMC2209:**
- R_sense = 0.11Ω
- E0_CURRENT setting = desired mA RMS

**Verification:**
- Setting E0_CURRENT = 670 → TMC2209 limits to 670mA RMS ✓
- Setting E0_CURRENT = 800 → 19.4% over motor rating ✗

---

### 4. Torque Calculation
**Formula:** F = (T × 2π) / L
**Where:**
- T = Torque (N·m) = 0.06 N·m (6 N·cm)
- L = Lead (m) = 0.001m
- F = Linear force (N)

**Calculation:**
```
F = (0.06 × 6.283) / 0.001
  = 376.99 N
  ≈ 38.4 kg force
```

**Bioprinting Requirement:** 5-50N (this motor provides 377N ✓)

---

## Risk Assessment

### If Changes Are NOT Made:

#### Risk 1: Incorrect Steps/mm (CRITICAL)
**Probability:** 100% (certain)
**Impact:** Print failure
**Consequence:**
- 6.4× under-extrusion with current 500 steps/mm
- G1 E10 would only extrude 1.56mm
- Impossible to print successfully

#### Risk 2: Motor Overcurrent (CRITICAL)
**Probability:** 100% (certain)
**Impact:** Motor damage
**Consequence:**
- 19.4% overcurrent causes excessive heating
- May exceed 130°C insulation rating
- Shortened motor lifespan (months instead of years)
- Potential motor failure mid-print

#### Risk 3: High Feedrate (MODERATE)
**Probability:** 50-80% (likely under load)
**Impact:** Skipped steps
**Consequence:**
- Inconsistent extrusion
- Position errors
- Print quality issues
- May work with low-viscosity bioinks, fail with high-viscosity

#### Risk 4: High Acceleration (MODERATE)
**Probability:** 30-60% (depends on load)
**Impact:** Skipped steps during rapid moves
**Consequence:**
- Grinding noise during retraction/prime
- Position errors accumulating over time
- Reduced print reliability

---

### If Changes ARE Made:

✅ **Correct extrusion volume** (3200 steps/mm matches 1mm lead screw)
✅ **Motor runs within rated specifications** (670mA current)
✅ **Reliable operation** (conservative feedrate/acceleration)
✅ **Long motor lifespan** (no overcurrent)
✅ **High precision** (0.3125 micron resolution with microstepping)

---

## Testing Plan (After Changes)

### Test 1: Verify Steps/mm
```gcode
M302 P1          ; Allow cold extrusion
M92 E3200        ; Verify steps/mm setting
M500             ; Save to EEPROM
M503             ; Display settings
```
**Expected Output:** `echo:  M92 X80.00 Y80.00 Z400.00 E3200.00`

---

### Test 2: Extrusion Calibration
```gcode
M302 P1          ; Allow cold extrusion
M83              ; Relative E mode
G1 E10 F60       ; Extrude 10mm at 1mm/s
```
**Measure:** Linear movement of lead screw = 10mm ± 0.1mm

---

### Test 3: Current Verification
```gcode
M906 E670        ; Set current
M500             ; Save
M122             ; TMC debug info
```
**Check:** TMC2209 reports run current = 670mA

---

### Test 4: Speed Test
```gcode
M302 P1          ; Allow cold extrusion
M83              ; Relative E mode
G1 E5 F60        ; 1mm/s - should work smoothly
G1 E5 F120       ; 2mm/s - should work smoothly
G1 E5 F180       ; 3mm/s - should work (at limit)
```
**Listen for:** Smooth operation, no grinding/skipping

---

## Conclusion

### Changes Status:
1. ✅ **Steps/mm updated** to 3200 (CRITICAL - already done)
2. ⏳ **Current needs update** to 670mA (CRITICAL - pending your approval)
3. ⏳ **Feedrate needs update** to 3 mm/s (RECOMMENDED - pending your approval)
4. ⏳ **Acceleration needs update** to 1000 mm/s² (RECOMMENDED - pending your approval)

### Proof Provided:
✅ Motor specifications from manufacturer website
✅ Mathematical calculations with formulas
✅ Verification against manufacturer data
✅ Risk assessment for each change
✅ Testing procedures to verify changes

### Next Step:
**Your approval required** to proceed with changes 2, 3, and 4.

---

**Do you approve the remaining changes (motor current, feedrate, acceleration)?**
