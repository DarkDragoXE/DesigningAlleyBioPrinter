# Bioprinter Firmware Configuration Session Summary
**Date:** 2026-01-08
**Repository:** https://github.com/DarkDragoXE/DesigningAlleyBioPrinter.git

---

## 1. UV LED PWM Control Implementation
**Commit:** `0dceb8c`

### Changes:
- **Pin 61 (PD13/FAN3)** configured for UV LED control via M42 commands
- Unprotected pin from sensitive pins list
- Initialized to OFF at startup

### G-Code Commands:
```gcode
M42 P61 S0      # UV LED OFF
M42 P61 S64     # 25% intensity
M42 P61 S128    # 50% intensity
M42 P61 S191    # 75% intensity
M42 P61 S255    # 100% full power
```

### Hardware Requirements:
- IRLZ44N MOSFET (logic-level N-channel)
- 1kΩ gate resistor, 10kΩ pull-down
- 1N4007 flyback diode
- UV LED array (365nm or 405nm)
- Separate 12V/24V power supply

### Files Modified:
1. **Configuration.h (lines 157-164):** CUSTOM_UV_LED_PIN definitions
2. **MarlinCore.cpp (lines 322-325):** Pin protection bypass
3. **MarlinCore.cpp (lines 1178-1182):** Pin initialization
4. **M42.cpp (lines 93-96):** Fan interception bypass

---

## 2. NEMA 11 Motor Configuration (E0 Extruder)
**Commit:** `2e3cef9`

### Motor Specifications:
- **Model:** JKongMotor NEMA 11 JK28HST32-0674
- **Lead Screw:** Tr5 × 1mm pitch (1mm lead)
- **Rated Current:** 670mA
- **Holding Torque:** 6 N·cm
- **Resolution:** 0.3125 microns with 16 microstepping

### Configuration Changes:
| Parameter | Old Value | New Value | Reason |
|-----------|-----------|-----------|--------|
| Steps/mm | 500 | **3200** | Tr5×1mm lead screw calculation |
| Motor Current | 800mA | **670mA** | Motor rated current (prevents damage) |
| Max Feedrate | 5 mm/s | **3 mm/s** | Lower torque motor |
| Max Acceleration | 2000 mm/s² | **1000 mm/s²** | Lower torque motor |

### Calculation:
```
Steps/mm = (200 steps/rev × 16 microsteps) / 1mm lead = 3200 steps/mm
```

### Files Modified:
1. **Configuration.h (line 1045):** Steps/mm updated
2. **Configuration.h (line 1052):** Max feedrate reduced
3. **Configuration.h (line 1065):** Max acceleration reduced
4. **Configuration_adv.h (line 2969):** Motor current set to 670mA

---

## 3. I and J Axes (Printhead Z Height Adjustment)
**Commit:** `892920d` + fixes

### Purpose:
- **I Axis (displayed as U):** Printhead 0 (E0) vertical positioning
- **J Axis (displayed as V):** Printhead 1 (E1) vertical positioning
- Independent control for multi-material printing and tool offset calibration

### Motor Assignment:
- **I Axis:** Motor 6 (BTT Octopus)
- **J Axis:** Motor 7 (BTT Octopus)
- **Motors:** NEMA 8 linear steppers (specs pending)
- **Drivers:** TMC2209

### Endstop Connectors:
- **I Axis (U):** E1DET connector (PG13)
- **J Axis (V):** E2DET connector (PG14)

### Configuration (Placeholders):
| Parameter | I Axis | J Axis | Status |
|-----------|--------|--------|--------|
| Steps/mm | 400 | 400 | Placeholder (pending motor specs) |
| Max Feedrate | 5 mm/s | 5 mm/s | Conservative |
| Max Acceleration | 100 mm/s² | 100 mm/s² | Conservative |
| Motor Current | 800mA | 800mA | Placeholder (adjust to motor rating) |
| Travel Range | 0-50mm | 0-50mm | Placeholder |

### G-Code Commands:
```gcode
# Homing (INDEPENDENT - not triggered by G28):
G28          # Homes X, Y, Z only (NOT I or J)
G28 I        # Home I axis only
G28 J        # Home J axis only

# Movement:
G1 I10 F300  # Move printhead 0 to 10mm
G1 J5 F300   # Move printhead 1 to 5mm

# Configuration:
M92 I400     # Set I steps/mm
M906 I500    # Set I current (500mA)
M500         # Save to EEPROM
```

### Files Modified:
1. **Configuration.h (lines 197-198):** I_DRIVER_TYPE and J_DRIVER_TYPE enabled
2. **Configuration.h (lines 229-236):** Axis names set to 'U' and 'V' (internal I/J)
3. **Configuration.h (line 1045):** Steps/mm array expanded
4. **Configuration.h (line 1052):** Feedrate array expanded
5. **Configuration.h (line 1065):** Acceleration array expanded
6. **Configuration.h (lines 1512-1513):** Homing direction set to MIN
7. **Configuration.h (lines 1532-1535):** I/J position limits added
8. **Configuration.h (lines 906-907):** USE_IMIN_PLUG and USE_JMIN_PLUG enabled
9. **Configuration_adv.h (lines 901-902):** Homing bump parameters added
10. **Configuration_adv.h (lines 2908-2925):** TMC2209 settings (already present)

---

## Summary of All Changes

### Motor Configuration:
| Motor | Slot | Purpose | Type | Current | Steps/mm | Status |
|-------|------|---------|------|---------|----------|--------|
| X | 0 | X axis | NEMA 17 | Default | 80 | ✅ Working |
| Y | 1 | Y axis | NEMA 17 | Default | 80 | ✅ Working |
| Z | 2 | Z axis | NEMA 17 | Default | 400 | ✅ Working |
| **E0** | **4** | **Syringe** | **NEMA 11** | **670mA** | **3200** | **✅ Configured** |
| **E1** | **5** | **Pneumatic** | **Valve** | **N/A** | **500** | **✅ Working** |
| **I (U)** | **6** | **Printhead 0 Z** | **NEMA 8** | **800mA** | **400** | **⏳ Pending specs** |
| **J (V)** | **7** | **Printhead 1 Z** | **NEMA 8** | **800mA** | **400** | **⏳ Pending specs** |

### Pin Assignments:
| Function | Pin | STM32 | Connector | Notes |
|----------|-----|-------|-----------|-------|
| UV LED PWM | 61 | PD13 | FAN3 | M42 P61 control |
| Peltier Polarity | 60 | PD12 | FAN2 | M42 P60 control |
| Pneumatic Valve | - | PC3 | E1_ENABLE | Automatic control |
| I Axis Endstop | - | PG13 | E1DET | Printhead 0 homing |
| J Axis Endstop | - | PG14 | E2DET | Printhead 1 homing |

### Pending Actions:

#### 1. NEMA 8 Motor Specifications (I and J Axes):
**Required Information:**
- Rated current (mA)
- Step angle (typically 1.8°)
- Lead screw pitch and lead (mm)
- Maximum travel range

**Once received, update:**
- Calculate correct steps/mm
- Set appropriate motor current
- Tune feedrate/acceleration
- Configure software endstops (I_MAX_POS, J_MAX_POS)

#### 2. Hardware Installation:
- Mount NEMA 8 motors in Motor 6 and Motor 7 slots
- Connect endstops to E1DET and E2DET connectors
- Build UV LED MOSFET circuit
- Wire UV LED array to 12V/24V power supply

#### 3. Firmware Testing:
- Rebuild firmware: `python -m platformio run -e STM32F446ZE_btt`
- Flash to BTT Octopus
- Test motor movements
- Test endstop triggering (M119)
- Calibrate axes

---

## Git Commits Summary

1. **`0dceb8c`** - UV LED PWM control implementation
2. **`2e3cef9`** - NEMA 11 motor configuration (E0)
3. **`892920d`** - I and J axes initial configuration
4. **`[pending]`** - I/J axes compilation fixes

---

## Next Steps

1. **Provide NEMA 8 motor product link/specifications**
2. **Build and flash firmware**
3. **Test all axes independently**
4. **Calibrate steps/mm for I and J after motor installation**
5. **Test G28 behavior (should NOT home I/J)**
6. **Wire endstops and test homing**

---

**End of Session Summary**
