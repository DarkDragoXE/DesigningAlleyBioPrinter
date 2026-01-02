# OctopusMarlin Bioprinter Project Memory

## Project Overview
This is a **bioprinter firmware development project** based on Marlin 2.0 bugfix branch, customized for the BTT Octopus V1.1 board. The system is designed for precision biological material deposition with advanced temperature control and multi-material dispensing capabilities.

**Primary Hardware:**
- Board: BTT Octopus V1.1 (STM32F446ZE)
- Firmware Base: Marlin 2.0.x bugfix branch
- Future Target: Migration to Klipper + T-Code system

## Project Goals & Active Work

### Current Implementation Plans
1. **Peltier Temperature Control** - Bidirectional heating/cooling for E0 extruder using DPDT relay + ULN2003 driver
   - See [PELTIER_TEMP_CONTROL_PLAN.md](PELTIER_TEMP_CONTROL_PLAN.md) for detailed implementation

2. **Klipper Migration** - Transitioning from Marlin to Klipper with T-Code support
   - See [KLIPPER_TCODE_IMPLEMENTATION_PLAN.md](KLIPPER_TCODE_IMPLEMENTATION_PLAN.md) for 12-week roadmap

3. **Configuration Optimization** - Bioprinter-specific tuning and calibration
   - See [BIOPRINTER_OPTIMIZED_VALUES_REPORT.md](BIOPRINTER_OPTIMIZED_VALUES_REPORT.md)
   - See [MARLIN_DEFAULT_VALUES_REPORT.md](MARLIN_DEFAULT_VALUES_REPORT.md)

## Build & Development Commands

### Build the firmware
```bash
pio run -e STM32F446ZE_btt
```

### Clean build
```bash
pio run -t clean
pio run -e STM32F446ZE_btt
```

### Monitor serial output
```bash
pio device monitor --baud 250000
```

### Run tests (if on Linux/WSL)
```bash
make tests-all-local
# Or specific test:
make tests-single-local TEST_TARGET=...
```

### Docker-based testing
```bash
make tests-all-local-docker
```

## Project Structure

### Key Directories
- **Marlin/** - Main firmware source code
  - **Marlin/src/** - Core firmware modules
  - **Marlin/Configuration.h** - Main configuration file
  - **Marlin/Configuration_adv.h** - Advanced configuration
- **config/** - Example configurations for various boards
- **buildroot/** - Build system and scripts
- **ini/** - PlatformIO environment configurations
- **docs/** - Technical documentation
- **.pio/** - PlatformIO build artifacts (ignored in git)

### Important Configuration Files
- [platformio.ini](platformio.ini) - Build environment configuration (default: STM32F446ZE_btt)
- **Marlin/Configuration.h** - Hardware-specific settings, pins, features
- **Marlin/Configuration_adv.h** - Advanced features and tuning parameters

## Code Standards & Conventions

### Language & Style
- **C/C++** for firmware code
- Follow Marlin coding standards: https://marlinfw.org/docs/development/coding_standards.html
- Use 2-space indentation (configured in .editorconfig)
- Document all public functions with clear comments
- Prefer descriptive variable names over abbreviations

### Safety-Critical Code
- **ALWAYS** validate temperature readings and bounds
- Include thermal runaway protection
- Implement failsafes for Peltier control (prevent simultaneous heating/cooling)
- Add bounds checking for all motion commands
- Test thoroughly with hardware simulation before deploying to physical hardware

### Testing Philosophy
- Write tests for new features in `buildroot/tests`
- Test with hardware simulation when possible
- Never commit untested code for safety-critical features
- Document test procedures in implementation plans

## Hardware-Specific Notes

### BTT Octopus V1.1 (STM32F446ZE)
- **Processor:** STM32F446ZE ARM Cortex-M4
- **Clock:** 180MHz
- **Flash:** 512KB
- **RAM:** 128KB
- **Logic Level:** 3.3V
- **FPU:** Yes

### Pin Assignments
Reference the specific pin mappings in:
- `Marlin/src/pins/stm32f4/pins_BTT_OCTOPUS_V1_common.h`

### Peltier Control System (Planned)
- **HEAT_PIN:** PWM control for heating relay
- **COOL_PIN:** PWM control for cooling relay
- **POLARITY_PIN:** Digital control for DPDT relay (polarity switching)
- **Driver:** ULN2003 driver array for relay control
- **Thermistor:** NTC 100K for E0 temperature sensing

## Git Workflow

### Branch Strategy
- Work on `bugfix-2.0.x` branch (Marlin standard)
- Create feature branches for major implementations
- Use descriptive commit messages following Marlin conventions

### Commit Message Format
```
Brief description of change (50 chars max)

More detailed explanation if needed. Reference issue numbers
or planning documents. Explain WHY the change was made.

Related: PELTIER_TEMP_CONTROL_PLAN.md
```

### Files to NEVER Commit
- `.pio/` build artifacts
- `nul` empty files
- Personal configuration files not in `.gitignore`
- Compiled binaries
- Credentials or API keys

## Common Issues & Solutions

### Build Issues
1. **Missing dependencies** - Run `pio pkg install` to install required libraries
2. **Board not found** - Verify `default_envs = STM32F446ZE_btt` in platformio.ini
3. **Upload fails** - Do NOT use `pio run -t upload` for STM32F446ZE_btt; use external programmer

### Configuration Issues
1. **Temperature reading errors** - Check thermistor table selection in Configuration.h
2. **Motion problems** - Verify steps/mm and acceleration settings
3. **Pin conflicts** - Cross-reference pins_BTT_OCTOPUS_V1_common.h

## Related Documentation

### Internal Docs
- [README.md](README.md) - Marlin overview and general information
- [Marlin to Klipper Mapping](../BTTOctopusKlipper/docs/MARLIN_TO_KLIPPER_MAPPING.md) - Configuration migration reference

### External Resources
- Marlin Documentation: https://marlinfw.org/
- BTT Octopus Manual: https://github.com/bigtreetech/BIGTREETECH-OCTOPUS-V1.0
- PlatformIO Docs: https://docs.platformio.org/
- STM32F446ZE Datasheet: https://www.st.com/resource/en/datasheet/stm32f446ze.pdf

## Development Environment

### Required Tools
- **PlatformIO** (installed via VSCode extension or CLI)
- **Git** for version control
- **VSCode** (recommended IDE) with extensions:
  - PlatformIO IDE
  - C/C++ IntelliSense
  - EditorConfig support

### Optional Tools
- **Arduino IDE 1.8.8+** (alternative to PlatformIO)
- **Docker** (for cross-platform testing)
- **Make** (for running test suites)

## Communication Protocol

### G-code Standards
- Marlin follows RepRap G-code standards
- Custom G-codes documented in `Marlin/src/gcode/gcode.h`

### Future T-Code Support
- T-Code will be implemented via Klipper macros
- Python-based control interface for bioprinting operations
- Reference: KLIPPER_TCODE_IMPLEMENTATION_PLAN.md

## Pneumatic Extruder Control (E1)

**NEW FEATURE - Implemented 2025-12-22**

### Overview
E1 has been converted from stepper motor control to pneumatic dispenser control for multi-material bioprinting.

### Pin Assignment
- **PC3 (E1_ENABLE_PIN)** - Pneumatic valve control signal
  - HIGH: Valve open → Material dispenses
  - LOW: Valve closed → No dispensing

### Usage
```gcode
T0        ; Select E0 (syringe stepper motor)
T1        ; Select E1 (pneumatic dispenser)
G1 E10 F180   ; When T1 active: PC3 goes HIGH for duration of move
```

### Implementation Files
- **Feature Code:** `Marlin/src/feature/pneumatic_extruder.h` & `.cpp`
- **Configuration:** `Configuration_adv.h` line 710 (`#define PNEUMATIC_EXTRUDER_E1`)
- **Stepper Integration:** `stepper.cpp` lines 2348-2355, `stepper.h` lines 524-531
- **Pin Definitions:** `indirection.h` lines 340-375
- **Testing Guide:** [PNEUMATIC_E1_TESTING_GUIDE.md](PNEUMATIC_E1_TESTING_GUIDE.md)

### Hardware Setup
- Connect PC3 (Motor 4 ENABLE pin) to pneumatic control board signal input
- Common ground between Octopus and pneumatic board
- Pneumatic pressure: 10-30 PSI typical for bioprinting

### Key Features
- Automatic valve control based on extrusion moves
- Seamless switching between T0 (stepper) and T1 (pneumatic)
- Valve timing matches G-code extrusion commands exactly
- No interference with E0 stepper motor operation
- Debug mode available (`#define DEBUG_PNEUMATIC_EXTRUDER`)

## Session History & Context

### Work Completed
- ✅ Initial Marlin configuration for BTT Octopus V1.1
- ✅ Peltier control system planning and architecture design
- ✅ Klipper migration roadmap development
- ✅ Configuration optimization analysis
- ✅ **Pneumatic extruder control for E1 (PC3 valve control)**

### Current Focus
- 🔄 Testing pneumatic E1 control with hardware
- 🔄 Peltier temperature control implementation
- 🔄 Preparing for Klipper migration

### Next Steps
- ⏭️ Calibrate pneumatic pressure for bioink dispensing
- ⏭️ Test multi-material switching (E0 ↔ E1)
- ⏭️ Test Peltier control system in simulation
- ⏭️ Implement safety interlocks for heating/cooling
- ⏭️ Begin Klipper environment setup

## Important Reminders for Claude

1. **Always read configuration files** before suggesting changes to pins, features, or hardware settings
2. **Check safety implications** for any temperature or motion control code
3. **Reference existing plans** (PELTIER_TEMP_CONTROL_PLAN.md, etc.) before proposing new approaches
4. **Test-driven development** - suggest tests for new features
5. **Document everything** - update this CLAUDE.md file when completing major milestones
6. **Cross-reference hardware** - BTT Octopus pin assignments must match firmware configuration

## Version Information
- **Marlin Version:** 2.0.x bugfix branch
- **PlatformIO:** Latest stable
- **Board Environment:** STM32F446ZE_btt
- **Last Updated:** 2025-12-22
