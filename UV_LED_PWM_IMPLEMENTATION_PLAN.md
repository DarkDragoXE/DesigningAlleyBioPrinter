# UV LED PWM Control - Complete Implementation Plan
**Project:** BTT Octopus V1.1 Bioprinter
**Feature:** G-code controlled UV crosslinking with PWM intensity control
**Status:** 📋 READY FOR IMPLEMENTATION (awaiting user approval)
**Date Prepared:** January 7, 2026

---

## 📋 Table of Contents

1. [Overview](#overview)
2. [Hardware Requirements](#hardware-requirements)
3. [Pin Selection](#pin-selection)
4. [Implementation Options](#implementation-options)
5. [Recommended Solution](#recommended-solution)
6. [Firmware Changes Required](#firmware-changes-required)
7. [Hardware Wiring Diagram](#hardware-wiring-diagram)
8. [G-code Commands](#g-code-commands)
9. [Slicer Integration](#slicer-integration)
10. [Testing Procedure](#testing-procedure)
11. [Safety Considerations](#safety-considerations)

---

## Overview

### What This Feature Does:
- ✅ UV LED controlled via G-code from slicer/software
- ✅ PWM intensity control (0-100% brightness)
- ✅ User sets duty cycle for precise crosslinking
- ✅ Automatic layer-by-layer UV exposure
- ✅ Safe, software-controlled operation

### How PWM Works:
```
PWM Signal (3.3V from BTT Octopus)
├─ 100% Duty Cycle → LED Full Brightness (always ON)
├─ 50% Duty Cycle  → LED Half Brightness (ON 50% of time)
├─ 25% Duty Cycle  → LED Quarter Brightness (ON 25% of time)
└─ 0% Duty Cycle   → LED OFF (never ON)

Frequency: ~490 Hz (invisible flicker to human eye)
```

---

## Hardware Requirements

### Components Needed:

#### 1. UV LED Array
- **Type:** 365nm or 405nm UV LEDs (for photopolymer crosslinking)
- **Power:** 12V or 24V (depending on LED strip specs)
- **Current:** Calculate based on LED array (e.g., 1-3A typical)
- **Mounting:** Position above print bed for uniform coverage

#### 2. MOSFET Driver
- **Part Number:** IRLZ44N (recommended)
  - **Alternative:** IRL540N, IRL520N, IRLB8721
- **Requirements:**
  - Logic-level (3.3V gate drive compatible)
  - N-channel MOSFET
  - Minimum 10A drain current rating
  - TO-220 package (easy heatsinking)
- **Quantity:** 1
- **Cost:** ~$1-2 per MOSFET

#### 3. Resistors
- **Gate Resistor:** 1kΩ (1/4W) - Between PWM pin and MOSFET gate
- **Pull-down Resistor (optional):** 10kΩ - MOSFET gate to GND (prevents floating)

#### 4. Diode (Flyback Protection)
- **Part Number:** 1N4007 or 1N5819 (Schottky, faster)
- **Purpose:** Protects MOSFET from inductive kickback
- **Installation:** Across UV LED load (cathode to +V, anode to GND)

#### 5. Power Supply
- **Voltage:** Match UV LED array (12V or 24V)
- **Current:** Match UV LED array + 20% safety margin
- **Example:** 24V 3A power supply for 24V LED array drawing 2A

#### 6. Wiring
- **PWM Signal Wire:** 22-24 AWG (low current, logic signal)
- **LED Power Wires:** 18-20 AWG (high current, 2-3A)
- **Connectors:** Screw terminals or JST-XH connectors

---

## Pin Selection

### Available PWM-Capable Pins on BTT Octopus V1.1:

| Pin | STM32 | Marlin Name | Current Use | **Recommendation** |
|-----|-------|-------------|-------------|-------------------|
| **PD13** | PD13 | FAN3_PIN | Unused | ✅ **BEST CHOICE** |
| **PD14** | PD14 | FAN4_PIN | Unused | ✅ Good backup |
| **PD15** | PD15 | FAN5_PIN | Unused | ✅ Good backup |
| PB10 | PB10 | HEATER_1 (HE1) | Unused | ⚠️ Heater pin (works but less intuitive) |
| PB11 | PB11 | HEATER_2 (HE2) | Unused | ⚠️ Heater pin |
| PA3 | PA3 | HEATER_BED | Unused | ❌ May need for heated bed |
| PA2 | PA2 | HEATER_0 | Peltier PWM | ❌ Already in use |
| PD12 | PD12 | FAN2_PIN | Peltier polarity | ❌ Already in use |

### **Recommended Pin: PD13 (FAN3)**

**Why PD13?**
- ✅ PWM-capable
- ✅ Not used by any current features
- ✅ Easy access on FAN3 connector
- ✅ Logical grouping (FAN pins for PWM devices)
- ✅ Pin number calculation: Port D (3) × 16 + 13 = **61**

**Physical Connector:** FAN3 on BTT Octopus V1.1
- **Location:** Near FAN2, between heater and motor connectors
- **Pinout:**
  - Pin 1: +24V (don't use, get power from separate supply)
  - Pin 2: GND (common ground)
  - Pin 3: **PD13 (PWM signal)** ← Use this

---

## Implementation Options

### Option 1: M42 Direct Pin Control ⭐ SIMPLEST
**Complexity:** Easy (no firmware changes)
**Control:** Direct PWM output via M42 command

**Pros:**
- ✅ No firmware modification needed
- ✅ Works immediately after wiring
- ✅ Full PWM control (0-255)

**Cons:**
- ⚠️ Requires manual G-code in slicer
- ⚠️ Pin number not intuitive (P61)

**G-code:**
```gcode
M42 P61 S255   ; UV LED 100% brightness
M42 P61 S128   ; UV LED 50% brightness
M42 P61 S0     ; UV LED off
```

---

### Option 2: Fan Mapping (M106/M107) ⭐⭐ RECOMMENDED
**Complexity:** Medium (minimal firmware changes)
**Control:** Standard fan control commands

**Pros:**
- ✅ Standard M106/M107 commands (familiar)
- ✅ Easy slicer integration
- ✅ Intuitive (M106 = turn on, M107 = turn off)
- ✅ Percentage-based (P0-255 or 0-100%)

**Cons:**
- ⚠️ Requires FAN3 to be enabled in firmware

**G-code:**
```gcode
M106 P2 S255   ; UV LED 100% (Fan index 2 = FAN3)
M106 P2 S128   ; UV LED 50%
M107 P2        ; UV LED off
```

---

### Option 3: Custom M355 Command ⭐⭐⭐ MOST PROFESSIONAL
**Complexity:** Hard (custom firmware code)
**Control:** Dedicated UV LED command

**Pros:**
- ✅ Dedicated M355 UV LED control (industry standard)
- ✅ Clear semantics (M355 = UV LED)
- ✅ Can add safety features (max exposure time)
- ✅ Can add to LCD menu

**Cons:**
- ⚠️ Requires custom firmware code
- ⚠️ More testing needed

**G-code:**
```gcode
M355 S1 P255   ; UV LED on, 100% brightness
M355 S1 P128   ; UV LED on, 50% brightness
M355 S0        ; UV LED off
```

---

## Recommended Solution

### **🎯 Phase 1: Start with M42 (Testing)**
→ **Phase 2: Implement Fan Mapping (Production)**
→ **Phase 3: Add M355 (Future Enhancement)**

---

## Firmware Changes Required

### **Phase 1: M42 Method (NO CHANGES NEEDED)**

**Status:** ✅ Ready to use immediately after hardware wiring
**Requirements:** None
**G-code:** `M42 P61 S<0-255>`

---

### **Phase 2: Fan Mapping Method** ⭐ RECOMMENDED

#### File 1: `Marlin/Configuration.h`

**Location:** Line ~605 (after extruder configuration)

**Add:**
```cpp
//===========================================================================
//========================= UV LED Configuration ============================
//===========================================================================

/**
 * UV LED Control for Photopolymer Crosslinking
 *
 * UV LED mapped to FAN3 (PD13) for PWM brightness control
 * Control via M106/M107 commands (fan index 2)
 *
 * Hardware:
 * - UV LED array (365nm or 405nm)
 * - MOSFET driver (IRLZ44N or similar)
 * - Connected to FAN3 connector (PD13 pin)
 *
 * Usage:
 *   M106 P2 S255   ; UV LED 100%
 *   M106 P2 S128   ; UV LED 50%
 *   M107 P2        ; UV LED off
 */
#define UV_LED_PIN  FAN3_PIN  // PD13 on BTT Octopus V1.1
```

#### File 2: `Marlin/Configuration.h`

**Location:** Line ~560 (fan configuration area)

**Modify:**
```cpp
// Number of fans
#define FAN_COUNT 3  // FAN0, FAN1, FAN2 (UV LED on FAN3 = index 2)
```

**Note:** If you already have fans defined, increment the count by 1.

---

### **Phase 3: Custom M355 Method** (Future)

When ready to implement, I have complete code prepared for:
- `Marlin/src/gcode/feature/M355_UV_LED.cpp` (new file)
- `Marlin/Configuration_adv.h` modifications
- G-code parser integration
- LCD menu integration (optional)

**Will implement when requested.**

---

## Hardware Wiring Diagram

### Circuit Schematic:

```
BTT Octopus V1.1 - FAN3 Connector
┌────────────────────────────────┐
│  Pin 1: +24V (not used)        │
│  Pin 2: GND  ──────────┐       │
│  Pin 3: PD13 (PWM)     │       │
└────────────┬───────────┘       │
             │                   │
             │ PWM Signal        │
             ↓                   │
       [1kΩ Resistor]            │
             ↓                   │
        MOSFET Gate              │
        (IRLZ44N)                │
             │                   │
    ┌────────┴────────┐          │
    │     Drain       │          │
    │                 │          │
    │     Source ─────┼──────────┘ (GND)
    └────────┬────────┘
             │
             ↓
      UV LED Array (-)
             │
       [Flyback Diode]
       (1N4007)
             │
             ↓
      UV Power Supply (+) 12V or 24V
             │
             ↓
      UV Power Supply (-) ─────→ Common GND
                                      ↓
                              BTT Octopus GND
```

### Detailed Connection Steps:

#### Step 1: MOSFET Connections
```
MOSFET IRLZ44N (TO-220 Package)
┌─────────────────┐
│  G   D   S      │  (looking at front, pins down)
│  │   │   │      │
└──┼───┼───┼──────┘
   │   │   │
   │   │   └─→ Connect to BTT Octopus GND (FAN3 Pin 2)
   │   │
   │   └─────→ Connect to UV LED Array (-)
   │
   └─────────→ Connect to PD13 via 1kΩ resistor (FAN3 Pin 3)

G = Gate (control signal)
D = Drain (high current path to load)
S = Source (ground return)
```

#### Step 2: UV LED Array Connections
```
UV LED Strip/Array
├─ (+) Positive → UV Power Supply (+) [12V or 24V]
└─ (-) Negative → MOSFET Drain (D)
```

#### Step 3: Flyback Diode (Protection)
```
1N4007 Diode
├─ Cathode (stripe end) → UV Power Supply (+)
└─ Anode (plain end)    → MOSFET Drain (= UV LED negative)

Purpose: Protects MOSFET from voltage spikes when LED turns off
```

#### Step 4: Power Supply
```
UV Power Supply (12V or 24V, 2-3A)
├─ (+) → UV LED Array (+)
└─ (-) → Common GND with BTT Octopus

CRITICAL: Share common ground between:
- BTT Octopus GND
- UV Power Supply GND
- MOSFET Source

Without common ground, PWM signal won't work correctly!
```

#### Step 5: Gate Resistor
```
BTT Octopus PD13 (FAN3 Pin 3)
    ↓
[1kΩ Resistor]
    ↓
MOSFET Gate (G)
    │
[10kΩ Resistor] ← Optional pull-down
    ↓
   GND

Pull-down resistor prevents floating gate when pin is uninitialized
```

---

### Physical Layout Recommendations:

#### Component Placement:
1. **MOSFET:** Mount on heatsink if LED array draws >1A
2. **Resistors:** Solder directly to MOSFET gate leg
3. **Flyback Diode:** Mount close to UV LED array
4. **Power Supply:** External, well-ventilated location
5. **UV LED Array:** Above print bed, 50-100mm height typical

#### Wire Gauges:
- **PWM Signal (PD13):** 24 AWG (low current)
- **GND to MOSFET Source:** 20 AWG (moderate current)
- **UV LED Power:** 18 AWG (high current, 2-3A)

#### Connector Options:
- **BTT Octopus Connection:** Use existing FAN3 2-pin connector
- **MOSFET Connections:** Screw terminals or solder directly
- **UV LED Array:** JST-SM or screw terminals

---

## G-code Commands

### Using M42 Method (Phase 1):

```gcode
; Turn UV LED on at full brightness
M42 P61 S255

; Turn UV LED on at 75% brightness
M42 P61 S191   ; 191 = 75% of 255

; Turn UV LED on at 50% brightness
M42 P61 S128   ; 128 = 50% of 255

; Turn UV LED on at 25% brightness
M42 P61 S64    ; 64 = 25% of 255

; Turn UV LED off
M42 P61 S0
```

### Using M106/M107 Method (Phase 2): ⭐ RECOMMENDED

```gcode
; Turn UV LED on at full brightness
M106 P2 S255

; Turn UV LED on at 75% brightness
M106 P2 S191

; Turn UV LED on at 50% brightness
M106 P2 S128

; Turn UV LED on at 25% brightness
M106 P2 S64

; Turn UV LED off
M107 P2
; OR
M106 P2 S0
```

### Using M355 Method (Phase 3 - Future):

```gcode
; Turn UV LED on at full brightness
M355 S1 P255

; Turn UV LED on at 50% brightness
M355 S1 P128

; Turn UV LED off
M355 S0

; Report UV LED status
M355
```

---

## Slicer Integration

### PrusaSlicer / SuperSlicer:

#### Layer Change G-code:
```gcode
; After each layer prints:
M106 P2 S255   ; UV LED on (100%)
G4 P5000       ; Wait 5 seconds (crosslink)
M107 P2        ; UV LED off
```

**Location in Slicer:**
- Printer Settings → Custom G-code → After layer change G-code

#### Start G-code (ensure UV off):
```gcode
M107 P2        ; Ensure UV LED is off at start
```

#### End G-code (ensure UV off):
```gcode
M107 P2        ; Turn off UV LED
```

---

### Cura:

#### Layer Change G-code:
Use "Post-Processing" → "Pause at Height" or custom script:

```python
# In Extensions → Post Processing → Modify G-Code
# Add "Search and Replace"
# Search for: ;LAYER:
# Replace with:
;LAYER:
M106 P2 S255
G4 P5000
M107 P2
```

---

### Manual G-code Integration:

For any slicer, insert directly in G-code file:

```gcode
G1 X10 Y10 Z1.0  ; Print first layer
; ... printing commands ...

; Layer 1 complete - UV crosslink
M106 P2 S255     ; UV LED on
G4 P5000         ; Wait 5 seconds
M107 P2          ; UV LED off

G1 Z1.2          ; Move to next layer
; ... continue printing ...
```

---

## Testing Procedure

### Phase 1: Hardware Verification (No Firmware Flash)

#### Test 1: MOSFET Wiring Check
**Tools:** Multimeter
**Steps:**
1. Power OFF BTT Octopus
2. Check continuity:
   - MOSFET Gate → PD13 (via 1kΩ resistor)
   - MOSFET Source → GND
   - MOSFET Drain → UV LED (-)
3. Check UV LED polarity:
   - UV LED (+) → Power supply (+)
   - UV LED (-) → MOSFET Drain

**Expected:** All connections verified ✅

---

#### Test 2: Manual MOSFET Test
**Tools:** Jumper wire
**Steps:**
1. Power OFF BTT Octopus
2. Connect MOSFET Gate directly to +3.3V (from BTT Octopus)
3. UV LED should turn ON (full brightness)
4. Disconnect Gate from +3.3V
5. UV LED should turn OFF

**Expected:** LED responds to gate voltage ✅

---

### Phase 2: Firmware Testing (After Wiring)

#### Test 3: PWM Signal Test
**Tools:** Multimeter (DC voltage mode)
**Steps:**
1. Flash firmware (if using Phase 2 method)
2. Power ON BTT Octopus
3. Send G-code: `M42 P61 S255`
4. Measure voltage at MOSFET Gate
5. **Expected:** ~3.3V (LED should be ON)
6. Send G-code: `M42 P61 S128`
7. **Expected:** ~1.65V average (LED dimmer)
8. Send G-code: `M42 P61 S0`
9. **Expected:** ~0V (LED OFF)

---

#### Test 4: PWM Duty Cycle Test
**G-code Test Sequence:**
```gcode
; Test 0% - LED should be OFF
M42 P61 S0
G4 P3000   ; Wait 3 seconds

; Test 25% - LED should be DIM
M42 P61 S64
G4 P3000

; Test 50% - LED should be MEDIUM
M42 P61 S128
G4 P3000

; Test 75% - LED should be BRIGHT
M42 P61 S191
G4 P3000

; Test 100% - LED should be FULL BRIGHTNESS
M42 P61 S255
G4 P3000

; Turn off
M42 P61 S0
```

**Expected:** LED brightness increases smoothly ✅

---

#### Test 5: Fan Command Test (Phase 2 Only)
```gcode
M106 P2 S255   ; UV LED 100%
G4 P2000
M106 P2 S128   ; UV LED 50%
G4 P2000
M107 P2        ; UV LED off
```

**Expected:** Same behavior as M42 test ✅

---

### Phase 3: Bioprinting Test

#### Test 6: Crosslinking Test
**Materials:** Photopolymer bioink, test substrate
**Steps:**
1. Print single layer (0.2mm height)
2. After layer completes: `M106 P2 S255` (UV on)
3. Wait 5 seconds: `G4 P5000`
4. Turn off UV: `M107 P2`
5. Inspect crosslinking

**Expected:** Bioink solidifies after UV exposure ✅

---

## Safety Considerations

### ⚠️ UV Safety - CRITICAL

#### Eye Protection:
- ✅ **MANDATORY:** UV safety glasses/goggles
- ✅ Block 365nm or 405nm wavelengths
- ✅ Wear whenever UV LED is powered on
- ❌ **NEVER look directly at UV LED**

#### Skin Protection:
- ✅ Minimize skin exposure to UV
- ✅ Use UV-blocking shields around print area
- ✅ Long sleeves recommended during operation

#### Enclosure:
- ✅ **HIGHLY RECOMMENDED:** Enclose UV source
- ✅ UV-blocking acrylic or polycarbonate
- ✅ Interlock switch (UV off when door opens)

---

### Electrical Safety:

#### Power Supply:
- ✅ Use proper UV LED power supply (12V/24V)
- ✅ Current rating > UV LED array draw + 20%
- ✅ Fuse protection recommended

#### Wiring:
- ✅ Proper wire gauges (see wiring diagram)
- ✅ Secure connections (no loose wires)
- ✅ Common ground between BTT Octopus and UV power

#### MOSFET Heatsinking:
- ✅ If LED array > 1A, mount MOSFET on heatsink
- ✅ Thermal paste between MOSFET and heatsink
- ✅ Monitor MOSFET temperature during operation

---

### Firmware Safety (Future - Phase 3):

#### Max Exposure Time Limit:
```cpp
#define UV_LED_MAX_EXPOSURE_MS  30000  // 30 seconds max continuous
```

#### Emergency Stop:
- M112 (emergency stop) should turn off UV LED immediately

#### Power Loss:
- UV LED should default to OFF on power-up

---

## Implementation Checklist

### Phase 1: M42 Method (Immediate Testing)

**Hardware:**
- [ ] Purchase MOSFET (IRLZ44N)
- [ ] Purchase resistors (1kΩ, 10kΩ)
- [ ] Purchase flyback diode (1N4007)
- [ ] Purchase UV LED array (365nm or 405nm)
- [ ] Purchase UV power supply (12V or 24V)
- [ ] Purchase wire (18-24 AWG)

**Wiring:**
- [ ] Connect PD13 (FAN3 Pin 3) to MOSFET Gate via 1kΩ resistor
- [ ] Connect MOSFET Source to BTT Octopus GND
- [ ] Connect MOSFET Drain to UV LED (-)
- [ ] Connect UV LED (+) to UV Power Supply (+)
- [ ] Connect UV Power Supply (-) to common GND
- [ ] Install flyback diode across UV LED load
- [ ] Double-check all connections

**Testing:**
- [ ] Manual MOSFET test (jumper wire to 3.3V)
- [ ] M42 P61 S255 test (full brightness)
- [ ] M42 P61 S128 test (50% brightness)
- [ ] M42 P61 S0 test (off)
- [ ] PWM duty cycle sweep test

**Documentation:**
- [ ] Take photos of wiring
- [ ] Label all connections
- [ ] Record UV LED specifications
- [ ] Record power supply specifications

---

### Phase 2: Fan Mapping (Production Setup)

**Firmware Modifications:**
- [ ] Modify `Marlin/Configuration.h` (add UV_LED_PIN definition)
- [ ] Modify `FAN_COUNT` to include UV LED fan
- [ ] Rebuild firmware: `pio run -e STM32F446ZE_btt`
- [ ] Flash new firmware to BTT Octopus
- [ ] Verify firmware flashed successfully

**Testing:**
- [ ] M106 P2 S255 test (full brightness)
- [ ] M106 P2 S128 test (50% brightness)
- [ ] M107 P2 test (off)
- [ ] Verify same behavior as M42 method

**Slicer Integration:**
- [ ] Add UV LED commands to layer change G-code
- [ ] Test single layer print with UV crosslinking
- [ ] Test multi-layer print with UV between layers
- [ ] Optimize exposure time for bioink

**Documentation:**
- [ ] Update firmware changelog
- [ ] Document slicer settings
- [ ] Create user guide for UV LED operation
- [ ] Commit firmware changes to GitHub

---

### Phase 3: M355 Custom Command (Future)

**When Ready to Implement:**
- [ ] Request M355 implementation
- [ ] Receive custom firmware code
- [ ] Integrate M355 into firmware
- [ ] Add LCD menu integration (optional)
- [ ] Test M355 commands
- [ ] Update documentation

---

## File Locations

### Configuration Files:
- `Marlin/Configuration.h` - UV LED pin definition, fan count
- `Marlin/Configuration_adv.h` - Advanced UV LED settings (Phase 3)

### Source Files (Phase 3):
- `Marlin/src/gcode/feature/M355_UV_LED.cpp` - M355 command implementation
- `Marlin/src/gcode/gcode.h` - G-code parser declaration

### Documentation:
- `UV_LED_PWM_IMPLEMENTATION_PLAN.md` - This file
- `CHANGELOG_2026-01-XX.md` - To be created when implemented

---

## Cost Estimate

| Component | Quantity | Unit Cost | Total |
|-----------|----------|-----------|-------|
| IRLZ44N MOSFET | 1 | $1.50 | $1.50 |
| 1kΩ Resistor | 1 | $0.10 | $0.10 |
| 10kΩ Resistor | 1 | $0.10 | $0.10 |
| 1N4007 Diode | 1 | $0.15 | $0.15 |
| UV LED Array (365nm) | 1 | $20-50 | $35.00 |
| 12V/24V Power Supply (3A) | 1 | $15-25 | $20.00 |
| Wire (assorted) | - | $5 | $5.00 |
| **Total Estimate** | | | **~$62** |

*Prices approximate, may vary by supplier*

---

## Summary

### What You Have Now:
✅ Complete implementation plan
✅ Hardware requirements list
✅ Detailed wiring diagrams
✅ G-code command reference
✅ Testing procedures
✅ Safety guidelines

### What's Ready:
✅ **Phase 1 (M42):** Ready to implement immediately (no firmware changes)
✅ **Phase 2 (Fan Mapping):** Firmware changes documented, ready to implement
⏳ **Phase 3 (M355):** Custom code prepared, ready when requested

### When You're Ready:
Just say: **"Implement UV LED control - Phase 1"** or **"Phase 2"**
I'll guide you through the exact steps!

---

## Questions Before Implementation?

- Which UV LED wavelength? (365nm or 405nm)
- What power supply voltage? (12V or 24V)
- Preferred control method? (M42 or M106/M107)
- UV LED placement above bed? (height/coverage area)
- Safety enclosure plans?

**Everything is documented and ready - just let me know when to proceed!**

---

**Document Status:** ✅ COMPLETE AND READY
**Prepared By:** Claude Code (Anthropic)
**Date:** January 7, 2026
**Next Action:** Awaiting user approval to implement
