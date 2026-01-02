# MARLIN DEFAULT VALUES - COMPLETE REPORT

**Date:** December 16, 2025
**Firmware:** Marlin 2.x (bugfix branch)
**Source:** [MarlinFirmware/Configurations](https://github.com/MarlinFirmware/Configurations/blob/release-2.0.9.3/config/default/Configuration.h)

---

## SPEED SETTINGS (Motion Velocity)

### DEFAULT_MAX_FEEDRATE
**Location:** Configuration.h:1039
**Default Value:** `{ 300, 300, 5, 25 }` mm/s
**Description:** Maximum speed limits for each axis (X, Y, Z, E0)

| Axis | Speed | Comment |
|------|-------|---------|
| X | 300 mm/s | 18,000 mm/min - Very fast for standard 3D printing |
| Y | 300 mm/s | 18,000 mm/min - Very fast for standard 3D printing |
| Z | 5 mm/s | 300 mm/min - Moderate Z speed |
| E0 | 25 mm/s | 1,500 mm/min - Fast extrusion |

### MAX_FEEDRATE_EDIT_VALUES
**Location:** Configuration.h:1043
**Default Value:** `{ 600, 600, 10, 50 }` mm/s
**Description:** Maximum user-editable speeds via M203 or LCD (2x DEFAULT_MAX_FEEDRATE)

---

## ACCELERATION SETTINGS (Rate of Speed Change)

### DEFAULT_MAX_ACCELERATION
**Location:** Configuration.h:1052
**Default Value:** `{ 3000, 3000, 100, 10000 }` mm/s²
**Description:** Maximum acceleration for each axis

| Axis | Acceleration | Comment |
|------|--------------|---------|
| X | 3000 mm/s² | Standard 3D printer acceleration |
| Y | 3000 mm/s² | Standard 3D printer acceleration |
| Z | 100 mm/s² | Conservative Z acceleration |
| E0 | 10000 mm/s² | Very high extruder acceleration |

**Time to reach max speed (X/Y):** 300 mm/s ÷ 3000 mm/s² = **0.1 seconds** (very aggressive)

### MAX_ACCEL_EDIT_VALUES
**Location:** Configuration.h:1056
**Default Value:** `{ 6000, 6000, 200, 20000 }` mm/s²
**Description:** Maximum user-editable accelerations (2x DEFAULT_MAX_ACCELERATION)

### DEFAULT_ACCELERATION
**Location:** Configuration.h:1067
**Default Value:** `3000` mm/s²
**Description:** Acceleration used for printing moves (X, Y, Z, E combined)

### DEFAULT_TRAVEL_ACCELERATION
**Location:** Configuration.h:1069
**Default Value:** `3000` mm/s²
**Description:** Acceleration for non-printing moves (travel/rapid positioning)
**Note:** Same as print acceleration - consistent motion

### DEFAULT_RETRACT_ACCELERATION
**Location:** Configuration.h:1068
**Default Value:** `3000` mm/s²
**Description:** Acceleration for extruder retraction moves

---

## JERK / JUNCTION DEVIATION (Instantaneous Speed Change)

**Note:** Marlin can use either Classic Jerk OR Junction Deviation (not both)

### CLASSIC_JERK (Default: DISABLED)
**Location:** Configuration.h:1079
**Status:** Commented out (`//#define CLASSIC_JERK`)
**Values if enabled:**
- DEFAULT_XJERK: 10.0 mm/s
- DEFAULT_YJERK: 10.0 mm/s
- DEFAULT_ZJERK: 0.3 mm/s
- DEFAULT_EJERK: 5.0 mm/s

### JUNCTION_DEVIATION (Default: ENABLED)
**Location:** Configuration.h:1109
**Default Value:** `0.013` mm
**Description:** Maximum deviation from true path at junctions
**Effect:** Higher values = faster corners but less accuracy
**Lower values:** More accurate corners but slower direction changes

---

## HOMING SETTINGS

### HOMING_FEEDRATE_MM_M
**Location:** Configuration.h:1907
**Default Value:** `{ (50*60), (50*60), (4*60) }` mm/min = `{ 3000, 3000, 240 }` mm/min
**In mm/s:** `{ 50, 50, 4 }` mm/s
**Description:** Speed at which axes approach endstops during homing

| Axis | Speed | Comment |
|------|-------|---------|
| X | 50 mm/s | Fast homing approach |
| Y | 50 mm/s | Fast homing approach |
| Z | 4 mm/s | Conservative Z homing |

### HOMING_BUMP_MM
**Location:** Configuration_adv.h:837
**Default Value:** `{ 5, 5, 2 }` mm
**Description:** Distance to back off after first endstop trigger before slow re-approach

### HOMING_BUMP_DIVISOR
**Location:** Configuration_adv.h:838
**Default Value:** `{ 2, 2, 4 }` (divisor)
**Description:** Speed reduction for second approach
**Result:** Second approach speeds = `{ 25, 25, 1 }` mm/s

### IMPROVE_HOMING_RELIABILITY
**Location:** Configuration_adv.h:3205
**Status:** **ENABLED** (custom modification)
**Effect:** Reduces acceleration to 100 mm/s² during homing, sets jerk to 0

---

## LCD MANUAL CONTROL SETTINGS

### MANUAL_FEEDRATE
**Location:** Configuration_adv.h:1272
**Default Value:** `{ 50*60, 50*60, 4*60, 60 }` mm/min = `{ 3000, 3000, 240, 60 }` mm/min
**In mm/s:** `{ 50, 50, 4, 1 }` mm/s
**Description:** Speed when manually jogging axes from LCD

| Axis | Speed | Comment |
|------|-------|---------|
| X | 50 mm/s | Fast manual positioning |
| Y | 50 mm/s | Fast manual positioning |
| Z | 4 mm/s | Conservative Z jogging |
| E | 1 mm/s | Very slow manual extrusion |

### FINE_MANUAL_MOVE
**Location:** Configuration_adv.h:1273
**Default Value:** `0.025` mm
**Description:** Smallest manual move increment for Z axis fine adjustment

### ENCODER_RATE_MULTIPLIER
**Location:** Configuration_adv.h:1281
**Status:** **DISABLED** (custom modification)
**Effect:** No speed increase when turning encoder knob quickly

---

## MOTION QUALITY SETTINGS

### S_CURVE_ACCELERATION
**Location:** Configuration.h:1122
**Status:** Commented out (`//#define S_CURVE_ACCELERATION`)
**Default:** Linear acceleration (not S-curve)

### JD_HANDLE_SMALL_SEGMENTS
**Location:** Configuration.h:1110
**Status:** **ENABLED** (active with Junction Deviation)
**Effect:** Uses curvature estimation for small segments with large junction angles

---

## SAFETY LIMITS

### LIMITED_MAX_FR_EDITING
**Location:** Configuration.h:1041
**Status:** **ENABLED**
**Effect:** Prevents user from setting speeds higher than MAX_FEEDRATE_EDIT_VALUES

### LIMITED_MAX_ACCEL_EDITING
**Location:** Configuration.h:1054
**Status:** **ENABLED**
**Effect:** Prevents user from setting acceleration higher than MAX_ACCEL_EDIT_VALUES

---

## SUMMARY - MOTION PROFILE ANALYSIS

### Speed Hierarchy (Fastest → Slowest)
1. **X/Y Max:** 300 mm/s (extreme)
2. **X/Y Homing:** 50 mm/s (fast)
3. **X/Y Manual:** 50 mm/s (fast)
4. **E Max:** 25 mm/s (moderate)
5. **Z Max:** 5 mm/s (moderate)
6. **Z Homing:** 4 mm/s (slow)
7. **Z Manual:** 4 mm/s (slow)
8. **E Manual:** 1 mm/s (very slow)

### Acceleration Consistency
- **Print moves:** 3000 mm/s²
- **Travel moves:** 3000 mm/s² ✅ Consistent
- **Retract moves:** 3000 mm/s² ✅ Consistent
- **Homing moves:** 100 mm/s² (via IMPROVE_HOMING_RELIABILITY)
- **Manual moves:** Uses DEFAULT_ACCELERATION (3000 mm/s²)

### Design Philosophy
- **Optimized for:** Fast plastic 3D printing (PLA, ABS, PETG)
- **Target speed:** High throughput
- **Acceleration:** Aggressive for quick direction changes
- **Corner handling:** Moderate precision (0.013mm deviation)

---

## ASSESSMENT FOR BIOPRINTING

### ❌ **NOT SUITABLE** for bioprinting due to:

1. **Speeds 10-30x too fast** for cell viability
2. **Acceleration 15-60x too aggressive** for delicate materials
3. **Motion jerks** would damage hydrogel structures
4. **Shear stress** would kill living cells

### ✅ **SUITABLE** for:
- Standard FDM 3D printing (plastics)
- Rapid prototyping
- Non-biological material extrusion

---

## NEXT STEPS

**Recommendation:** Replace with bioprinter-optimized values that ensure:
1. Consistent slow speeds across all motion types
2. Gentle acceleration (100-200 mm/s²)
3. Tight junction control for precision
4. All parameters harmonized together

---

**End of Report**
