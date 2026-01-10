# NEMA 11 Linear Stepper Motor Compatibility Report
**Motor:** JKongMotor NEMA 11 JK28HST32-0674 Linear Stepper Motor
**Application:** E0 Extruder (Bioprinter Syringe Pump)
**Driver:** TMC2209
**Date:** 2026-01-08

---

## Executive Summary

✅ **COMPATIBLE** - The NEMA 11 JK28HST32-0674 linear stepper motor will work with your BTT Octopus V1.1 and TMC2209 driver.

⚠️ **CRITICAL CONFIGURATION CHANGES REQUIRED:**
1. **Motor current must be reduced** from 800mA to 670mA (motor rated current)
2. **Steps/mm must be recalculated** based on lead screw specification (you need to specify which lead screw you ordered)
3. **Feedrate and acceleration may need adjustment** due to lower torque

---

## Motor Specifications (From JKongMotor Website)

### Electrical Specifications
| Parameter | Value | Notes |
|-----------|-------|-------|
| **Step Angle** | 1.8° | Same as standard NEMA 17 (200 steps/rev) |
| **Rated Current** | **0.67A (670mA)** | ⚠️ Lower than current config (800mA) |
| **Resistance** | 5.6Ω ± 0.1Ω @ 20°C | Higher than typical NEMA 17 (~2-3Ω) |
| **Inductance** | 3.4mH ± 20% @ 1kHz | Similar to NEMA 17 |
| **Holding Torque** | 6 N·cm (8.5 oz·in) | ⚠️ Much lower than NEMA 17 (~26-40 N·cm) |
| **Phase** | 2-phase bipolar | Compatible with TMC2209 |
| **Leads** | 4 wires | Standard bipolar configuration |
| **Insulation Class** | B (130°C max) | Standard |

### Physical Specifications
| Parameter | Value |
|-----------|-------|
| **Frame Size** | NEMA 11 (28mm × 28mm) |
| **Body Length** | 32mm |
| **Shaft Type** | External T-type Lead Screw |
| **Shaft Diameter** | Tr4.77 / Tr5 / Tr5.56 / Tr6 / Tr6.35 / Tr8 (multiple options) |
| **Lead Length** | 300mm |
| **Weight** | 0.11 kg (110g) |
| **Rotor Inertia** | 9 g·cm² |

---

## Lead Screw Specifications

**CRITICAL:** You must specify which lead screw diameter and pitch you ordered. This determines steps/mm calculation.

### Available Lead Screw Options

| Diameter | Pitch (mm) | Lead (mm) | Travel/Step (mm) | Steps/mm | Use Case |
|----------|-----------|-----------|------------------|----------|----------|
| **Tr8** | 2 | 2 | 0.01 | **100** | ⭐ **Recommended for bioprinting** (good balance) |
| Tr8 | 4 | 2 | 0.02 | 50 | Faster extrusion, less precision |
| Tr8 | 8 | 2 | 0.04 | 25 | Very fast, low precision |
| Tr8 | 12 | 2 | 0.06 | 16.67 | Fastest, lowest precision |
| **Tr6** | 1 | 1 | 0.005 | **200** | Higher precision, slower |
| Tr6 | 4 | 1 | 0.02 | 50 | Fast, moderate precision |
| Tr6 | 6 | 1 | 0.03 | 33.33 | Faster, lower precision |
| Tr6 | 12 | 1.5 | 0.06 | 16.67 | Very fast, low precision |
| Tr5 | 1 | 1 | 0.005 | 200 | High precision |
| Tr5 | 2 | 1 | 0.01 | 100 | Balanced |
| Tr5 | 4 | 1 | 0.02 | 50 | Fast |
| Tr5 | 8 | 1 | 0.04 | 25 | Very fast |
| Tr6.35 | 1.27 | 1.27 | 0.006 | 166.67 | Imperial standard (1/20") |
| Tr6.35 | 1.5875 | 1.5875 | 0.008 | 125 | Imperial (1/16") |
| Tr4.77 | 0.635 | 0.635 | 0.003 | 333.33 | Ultra-high precision |
| Tr4.77 | 1.27 | 1.27 | 0.006 | 166.67 | High precision |

**Note:** Steps/mm calculations assume:
- 200 steps/revolution (1.8° step angle)
- 16 microstepping (TMC2209 default)
- Formula: Steps/mm = (200 steps/rev × 16 microsteps) / Lead (mm)

### Recommended Lead Screw for Bioprinting

**⭐ Tr8 with 2mm pitch (2mm lead):**
- **Steps/mm:** 1600 (with 16 microstepping)
- **Travel per step:** 0.01mm (10 microns with microstepping)
- **Why:** Good balance of precision and speed for bioink extrusion
- **Torque requirement:** Lower than high-pitch options, motor can handle it

---

## Current Firmware Configuration (E0 Motor)

### From Configuration.h
```cpp
#define E0_DRIVER_TYPE TMC2209                        // ✅ Correct
#define DEFAULT_AXIS_STEPS_PER_UNIT { 80, 80, 400, 500, 500 }  // E0 = 500 steps/mm
#define DEFAULT_MAX_FEEDRATE { 30, 30, 5, 5 }         // E0 = 5 mm/s
#define DEFAULT_MAX_ACCELERATION { 150, 150, 50, 2000 }  // E0 = 2000 mm/s²
```

### From Configuration_adv.h
```cpp
#define E0_CURRENT      800    // ⚠️ TOO HIGH - Motor rated at 670mA
#define E0_MICROSTEPS    16    // ✅ Correct for TMC2209
#define E0_RSENSE      0.11    // ✅ Correct for TMC2209
```

---

## TMC2209 Driver Compatibility

### ✅ Fully Compatible

| Feature | TMC2209 | NEMA 11 JK28HST32-0674 | Compatible? |
|---------|---------|------------------------|-------------|
| **Motor Type** | 2-phase bipolar | 2-phase bipolar | ✅ Yes |
| **Current Range** | 0-2A RMS | 0.67A rated | ✅ Yes (within range) |
| **Voltage** | 4.75-29V | Not specified, typical 12-24V | ✅ Yes |
| **Microstepping** | Up to 256 | Works with any microstepping | ✅ Yes |
| **StealthChop** | Yes | Works with any stepper | ✅ Yes |
| **Stallguard** | Yes | Works with any stepper | ✅ Yes |
| **Pin Configuration** | 4-wire bipolar | 4-wire bipolar | ✅ Yes |

**Conclusion:** TMC2209 is fully compatible with this motor.

---

## Comparison: Current vs New Motor

### Electrical Comparison

| Parameter | Current (NEMA 17 typical) | New (NEMA 11 JK28HST32-0674) | Impact |
|-----------|---------------------------|------------------------------|--------|
| **Rated Current** | ~1.5A | **0.67A** | ⚠️ Must reduce current setting |
| **Resistance** | ~2-3Ω | **5.6Ω** | Higher resistance, less heat |
| **Inductance** | ~3-5mH | **3.4mH** | Similar performance |
| **Holding Torque** | ~26-40 N·cm | **6 N·cm** | ⚠️ 77-85% less torque |

### Physical Comparison

| Parameter | NEMA 17 | NEMA 11 | Impact |
|-----------|---------|---------|--------|
| **Frame Size** | 42mm × 42mm | **28mm × 28mm** | Smaller footprint |
| **Weight** | ~300-400g | **110g** | 65-73% lighter |
| **Mounting** | Standard NEMA 17 holes | **Different hole pattern** | ⚠️ Mounting bracket change required |

### Performance Impact

| Aspect | Impact | Notes |
|--------|--------|-------|
| **Torque** | ⚠️ **Much Lower** | 77-85% less torque - may struggle with high-viscosity bioinks |
| **Precision** | ✅ **Same** | Same 1.8° step angle, linear screw provides precision |
| **Speed** | ⚠️ **May be slower** | Lower torque limits max acceleration/feedrate |
| **Weight** | ✅ **Better** | Lighter motor, less inertia |
| **Power** | ✅ **Lower** | Less current = less heat = longer life |

---

## Required Firmware Changes

### ⚠️ CRITICAL: Motor Current Reduction

**Current Setting (Configuration_adv.h line 2969):**
```cpp
#define E0_CURRENT      800  // ⚠️ TOO HIGH FOR NEMA 11
```

**Required Change:**
```cpp
#define E0_CURRENT      670  // BIOPRINTER: NEMA 11 JK28HST32-0674 rated current (0.67A)
```

**Why:**
- Motor rated current: 0.67A (670mA)
- Current setting of 800mA is **19% overcurrent**
- **Risk:** Motor overheating, reduced lifespan, potential damage
- TMC2209 will limit current to this value via current sensing

**Safety Factor:**
- You could run at 80-90% of rated current for longevity: 540-600mA
- For bioprinting (low duty cycle), 670mA is acceptable

---

### ⚠️ CRITICAL: Steps/mm Recalculation

**You MUST specify which lead screw you ordered** to calculate correct steps/mm.

**Current Setting (Configuration.h line 1046):**
```cpp
#define DEFAULT_AXIS_STEPS_PER_UNIT { 80, 80, 400, 500, 500 }  // E0 = 500 steps/mm
```

**Formula:**
```
Steps/mm = (Motor steps/rev × Microstepping) / Lead (mm/rev)
         = (200 × 16) / Lead
         = 3200 / Lead
```

**Example Calculations (assuming 16 microstepping):**

| Lead Screw | Lead (mm) | Steps/mm Calculation | Required Setting |
|------------|-----------|----------------------|------------------|
| **Tr8 × 2mm** (Recommended) | 2 | 3200 / 2 = **1600** | `{ 80, 80, 400, 1600, 500 }` |
| Tr8 × 4mm | 4 | 3200 / 4 = 800 | `{ 80, 80, 400, 800, 500 }` |
| Tr8 × 8mm | 8 | 3200 / 8 = 400 | `{ 80, 80, 400, 400, 500 }` |
| Tr6 × 1mm | 1 | 3200 / 1 = 3200 | `{ 80, 80, 400, 3200, 500 }` |
| Tr6 × 4mm | 4 | 3200 / 4 = 800 | `{ 80, 80, 400, 800, 500 }` |
| Tr5 × 2mm | 2 | 3200 / 2 = 1600 | `{ 80, 80, 400, 1600, 500 }` |

**⚠️ If you use the wrong steps/mm:**
- Too high → Under-extrusion (not enough bioink)
- Too low → Over-extrusion (too much bioink, wastage)

---

### ⚙️ RECOMMENDED: Feedrate/Acceleration Adjustment

Due to **77-85% lower torque**, you may need to reduce max feedrate and acceleration.

**Current Settings (Configuration.h):**
```cpp
#define DEFAULT_MAX_FEEDRATE { 30, 30, 5, 5 }         // E0 = 5 mm/s
#define DEFAULT_MAX_ACCELERATION { 150, 150, 50, 2000 }  // E0 = 2000 mm/s²
```

**Recommended Changes (Conservative):**
```cpp
#define DEFAULT_MAX_FEEDRATE { 30, 30, 5, 3 }         // E0 = 3 mm/s (reduce from 5)
#define DEFAULT_MAX_ACCELERATION { 150, 150, 50, 1000 }  // E0 = 1000 mm/s² (reduce from 2000)
```

**Why:**
- Lower torque motor may skip steps at high speeds/accelerations
- Bioprinting typically uses low speeds anyway (cell viability)
- You can test and increase if motor performs well

**Testing:**
1. Start with conservative values
2. Test extrusion at different speeds (M83, G1 E10 F100/F200/F300)
3. Listen for motor skipping/grinding
4. Increase feedrate gradually until motor struggles
5. Set max feedrate to 80% of struggling point

---

## Pin Compatibility

### ✅ No Pin Changes Required

The NEMA 11 motor uses the same electrical interface as NEMA 17:
- **4-wire bipolar configuration**
- **2-phase stepper motor**
- **Standard A1, A2, B1, B2 coil connections**

**Current E0 connections on BTT Octopus V1.1:**
- Already configured for TMC2209 driver
- Same pinout works for NEMA 11

**No firmware changes needed for pins.**

---

## Mechanical Compatibility

### ⚠️ Mounting Bracket Change Required

| Parameter | NEMA 17 | NEMA 11 | Impact |
|-----------|---------|---------|--------|
| **Faceplate Size** | 42mm × 42mm | **28mm × 28mm** | Different mounting holes |
| **Mounting Holes** | 31mm spacing | **23mm spacing** | ⚠️ New bracket required |
| **Shaft Type** | Rotary (5mm or 8mm) | **Linear lead screw** | ⚠️ Different coupling method |

**You will need:**
1. **New motor mount** for 28mm × 28mm NEMA 11 faceplate
2. **No coupler needed** - lead screw is integrated into motor shaft
3. **Linear guide for syringe plunger** - lead screw pushes plunger directly

**Advantages of integrated lead screw:**
- No coupler backlash
- Simpler mechanical design
- Direct linear motion (no rotary-to-linear conversion)

---

## Performance Predictions

### Torque Analysis

**Motor Holding Torque:** 6 N·cm = 0.06 N·m

**Force at Lead Screw (Tr8 × 2mm lead):**
```
Force = (Torque × 2π) / Lead
      = (0.06 N·m × 2π) / 0.002m
      = 188.5 N (~19.2 kg force)
```

**For comparison, typical bioprinting syringe pump forces:**
- **Low viscosity bioink:** 5-20 N (this motor is fine ✅)
- **Medium viscosity bioink:** 20-50 N (this motor may struggle ⚠️)
- **High viscosity bioink:** 50-100+ N (this motor is insufficient ❌)

**Conclusion:**
- ✅ **Good for:** Low to medium viscosity bioinks (alginate, gelatin, collagen at low concentrations)
- ⚠️ **Marginal for:** Medium-high viscosity bioinks (may need pressure assistance)
- ❌ **Not suitable for:** High viscosity bioinks without pneumatic assist

---

## Summary of Required Changes

### 🔴 CRITICAL (Must Do Before Using Motor)

1. **Reduce Motor Current:**
   - File: `Configuration_adv.h` line 2969
   - Change: `#define E0_CURRENT 800` → `#define E0_CURRENT 670`
   - Why: Prevent motor overcurrent and damage

2. **Recalculate Steps/mm:**
   - File: `Configuration.h` line 1046
   - Change depends on your lead screw specification
   - **You must tell me which lead screw you ordered** (Tr8×2mm? Tr6×1mm? etc.)
   - Example (Tr8×2mm): `{ 80, 80, 400, 500, 500 }` → `{ 80, 80, 400, 1600, 500 }`

### 🟡 RECOMMENDED (Should Do)

3. **Reduce Max Feedrate:**
   - File: `Configuration.h` line 1053
   - Change: `{ 30, 30, 5, 5 }` → `{ 30, 30, 5, 3 }`
   - Why: Lower torque motor, prevent skipping

4. **Reduce Max Acceleration:**
   - File: `Configuration.h` line 1066
   - Change: `{ 150, 150, 50, 2000 }` → `{ 150, 150, 50, 1000 }`
   - Why: Lower torque motor, prevent skipping

### 🟢 OPTIONAL (Nice to Have)

5. **Enable TMC StealthChop:**
   - Already may be enabled, check Configuration_adv.h
   - Quieter operation, good for low-speed bioprinting

6. **Tune Acceleration:**
   - After testing, fine-tune feedrate/acceleration based on actual performance

---

## Testing Procedure

### Test 1: Motor Connection Test
```gcode
M302 P1          ; Allow cold extrusion (for testing)
M83              ; Relative extrusion mode
G1 E10 F60       ; Extrude 10mm at 1mm/s
```
**Expected:** Motor should rotate smoothly, no grinding/skipping

---

### Test 2: Speed Test
```gcode
M302 P1          ; Allow cold extrusion
M83              ; Relative extrusion mode
G1 E10 F60       ; 1mm/s - should work fine
G1 E10 F120      ; 2mm/s - should work fine
G1 E10 F180      ; 3mm/s - may work (depends on load)
G1 E10 F240      ; 4mm/s - may skip if torque insufficient
```
**Expected:** Find maximum reliable speed before skipping occurs

---

### Test 3: Torque Test (With Syringe)
```gcode
M302 P1          ; Allow cold extrusion
M83              ; Relative extrusion mode
G1 E5 F60        ; Extrude with actual bioink loaded
```
**Expected:**
- ✅ Smooth extrusion = torque is sufficient
- ⚠️ Jerky motion = marginal torque
- ❌ Grinding/skipping = insufficient torque (need lower viscosity or pressure assist)

---

### Test 4: Precision Test
```gcode
M302 P1          ; Allow cold extrusion
M83              ; Relative extrusion mode
G1 E1 F60        ; Extrude exactly 1mm
; Measure actual extrusion volume
; Calculate: Actual steps/mm = (Configured steps/mm × Expected volume) / Actual volume
```
**Expected:** Verify steps/mm calibration is correct

---

## Troubleshooting

### Issue 1: Motor Overheating
**Symptoms:** Motor gets very hot during operation

**Causes:**
- Current set too high (800mA instead of 670mA)
- Motor running continuously at high duty cycle

**Solutions:**
- ✅ Reduce E0_CURRENT to 670mA or lower (540-600mA)
- ✅ Enable TMC hold current reduction (E0_HOLD_MULTIPLIER = 0.5)
- Check for mechanical binding (lead screw friction)

---

### Issue 2: Motor Skipping Steps
**Symptoms:** Motor makes grinding noise, doesn't move properly

**Causes:**
- Feedrate too high for low torque motor
- Acceleration too high
- Bioink viscosity too high (load exceeds motor torque)
- Lead screw binding or misaligned

**Solutions:**
- ✅ Reduce feedrate (try 1-2 mm/s instead of 5 mm/s)
- ✅ Reduce acceleration (try 500-1000 mm/s² instead of 2000 mm/s²)
- Use lower viscosity bioink
- Add pneumatic pressure assist
- Lubricate lead screw
- Check mechanical alignment

---

### Issue 3: Under-Extrusion / Over-Extrusion
**Symptoms:** Wrong amount of bioink dispensed

**Causes:**
- Steps/mm not calibrated correctly for lead screw

**Solutions:**
- ✅ **Tell me which lead screw you have** so I can calculate exact steps/mm
- Perform extrusion calibration test (Test 4 above)
- Adjust steps/mm based on actual measured extrusion

---

### Issue 4: Motor Not Moving At All
**Symptoms:** Motor doesn't respond to G-code commands

**Causes:**
- Current set to 0 or too low
- Wiring incorrect
- Driver not enabled

**Solutions:**
- ✅ Verify E0_CURRENT = 670 (not 0)
- Check 4-wire motor connections (A1, A2, B1, B2)
- Verify TMC2209 driver is seated properly
- Send M122 to check TMC driver status

---

## Next Steps

### Before You Build Firmware:

**🔴 CRITICAL: I need you to tell me which lead screw specification you ordered:**

Please check your motor order and tell me:
1. **Screw diameter:** Tr4.77 / Tr5 / Tr5.56 / Tr6 / Tr6.35 / Tr8?
2. **Pitch:** What pitch option did you select?
3. **Lead:** What is the lead value?

**Example:** "I ordered Tr8 with 2mm pitch and 2mm lead"

Once you tell me, I will:
1. Calculate the exact steps/mm for your configuration
2. Update Configuration.h with the correct value
3. Provide you with the exact firmware changes needed

---

### After You Provide Lead Screw Specs:

1. **I will update firmware files:**
   - Configuration.h (steps/mm, feedrate, acceleration)
   - Configuration_adv.h (motor current)

2. **You will rebuild firmware:**
   ```bash
   pio run -e STM32F446ZE_btt
   ```

3. **Flash to BTT Octopus**

4. **Run testing procedures above**

5. **Calibrate based on actual extrusion results**

---

## Conclusion

### ✅ Motor IS Compatible

The NEMA 11 JK28HST32-0674 will work with your BTT Octopus V1.1 and TMC2209 driver.

### ⚠️ Critical Action Required

**You MUST provide lead screw specifications before I can complete firmware configuration.**

### 🎯 Recommended Lead Screw (if you haven't ordered yet)

**Tr8 × 2mm pitch (2mm lead):**
- Steps/mm: 1600 (good precision)
- Travel per step: 0.01mm with microstepping
- Force capability: ~188N (good for low-medium viscosity bioinks)
- Balanced performance for bioprinting

---

**Please reply with your lead screw specification so I can complete the firmware configuration!**
