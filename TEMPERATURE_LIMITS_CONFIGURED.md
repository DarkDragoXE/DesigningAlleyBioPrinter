# Peltier Temperature Configuration - BTTOctopusDebKeshava
**Date Configured:** 2026-01-07
**Configuration Type:** Manual Peltier Control via M42 P60

---

## Temperature Limits Applied

### E0 Hotend (Peltier Module)
```cpp
HEATER_0_MINTEMP:    1°C   // Minimum cooling temperature
HEATER_0_MAXTEMP:   60°C   // Maximum heating temperature
HOTEND_OVERSHOOT:    5°C   // Safety buffer (max settable: 55°C)
```

### Safety Ranges
- **Cooling Mode (M42 P60 S0):**  Can cool down to 1°C minimum
- **Heating Mode (M42 P60 S255):** Can heat up to 55°C maximum (60°C hard limit)
- **Firmware Protection:** Any temperature command outside 1-60°C will trigger MAXTEMP/MINTEMP error

---

## How It Works

### 1. Set Polarity Mode FIRST
```gcode
M42 P60 S255   ; Set HEATING mode (PD12 HIGH → DPDT energized)
; OR
M42 P60 S0     ; Set COOLING mode (PD12 LOW → DPDT relaxed)
```

### 2. Then Set Target Temperature
```gcode
; For HEATING (after M42 P60 S255):
M104 S37       ; Heat to 37°C (body temperature)
M109 S50       ; Heat to 50°C and wait

; For COOLING (after M42 P60 S0):
M104 S4        ; Cool to 4°C (sample preservation)
M109 S10       ; Cool to 10°C and wait
```

### 3. Monitor Temperature
```gcode
M105           ; Report current temperature
```

---

## Usage Examples

### Bioprinting at Body Temperature
```gcode
M42 P60 S255   ; Enable heating mode
M109 S37       ; Heat to 37°C and wait
G28 E          ; Home syringe to refill position
; <Manual refill>
G1 E100 F300   ; Move away from refill
G92 E0         ; Zero position
; Start printing at 37°C
```

### Sample Preservation
```gcode
M42 P60 S0     ; Enable cooling mode
M109 S4        ; Cool to 4°C and wait
M117 Sample preserved
```

### Room Temperature Operation
```gcode
M42 P60 S255   ; Set heating mode
M104 S25       ; Set to room temp (no heating needed)
; OR
M42 P60 S0     ; Set cooling mode  
M104 S25       ; Set to room temp (no cooling needed)
```

---

## Safety Features

### Firmware Protection
✅ Cannot heat above 60°C (MAXTEMP shutdown)
✅ Cannot cool below 1°C (MINTEMP shutdown)
✅ PID control prevents temperature oscillation
✅ Overshoot protection: Max settable = 55°C

### Hardware Protection
✅ DPDT relay prevents simultaneous heating/cooling
✅ M42 P60 manual control prevents mode conflicts
✅ Thermistor monitors actual temperature

### Recommended Practices
1. **Always set M42 P60 BEFORE setting temperature**
2. **For heating:** M42 P60 S255 THEN M104 S(temp)
3. **For cooling:** M42 P60 S0 THEN M104 S(temp)
4. **Never exceed 55°C** in bioprinting (cell death risk)
5. **PID tune at your target temperature:** M303 E0 S37 C8

---

## PID Tuning Commands

### Initial Calibration
```gcode
; For heating at 37°C:
M42 P60 S255           ; Set heating mode
M303 E0 S37 C8         ; Auto-tune PID for 37°C, 8 cycles
M500                   ; Save PID values to EEPROM

; For cooling at 4°C:
M42 P60 S0             ; Set cooling mode
M303 E0 S4 C8          ; Auto-tune PID for 4°C
M500                   ; Save values
```

### View Current PID Values
```gcode
M503                   ; Report all settings including PID
```

---

## Current PID Settings
```cpp
DEFAULT_Kp: 22.20
DEFAULT_Ki:  1.08  
DEFAULT_Kd: 114.00
```
**Note:** These are default values from Keshavafirmware. Run M303 auto-tune for your specific Peltier module.

---

## Temperature Capabilities

| Mode | Min Temp | Target Range | Max Temp | Use Case |
|------|----------|--------------|----------|----------|
| **Cooling** | 1°C | 1-25°C | 25°C | Sample preservation, cold storage |
| **Ambient** | 25°C | 20-30°C | 30°C | Room temperature dispensing |
| **Heating** | 30°C | 30-55°C | 55°C | Body temp (37°C), warm gelation |

---

## Build & Flash Instructions

### 1. Rebuild Firmware
```bash
cd /c/BIOPRINTER/BTTOctopusDebKeshava/OctopusMarlin-bugfix-test
pio run -e STM32F446ZE_btt
```

### 2. Flash to BTT Octopus
- Copy `.pio/build/STM32F446ZE_btt/firmware.bin` to SD card
- Rename to `firmware.cur` (if required by your bootloader)
- Insert SD card and power cycle board

### 3. Verify After Flash
```gcode
M503           ; Check all settings loaded
M105           ; Check temperature reading
M42 P60 S255   ; Test heating mode control
M42 P60 S0     ; Test cooling mode control
```

---

**Configuration Status:** ✅ READY FOR BIOPRINTING
**Next Steps:** Rebuild firmware, flash, and test temperature control
