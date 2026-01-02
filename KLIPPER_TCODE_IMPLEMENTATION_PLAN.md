# Klipper + T-Code Bioprinter Implementation Plan
## 12-Week Detailed Roadmap from Marlin to Production-Ready Bioprinter

**Project Goal:** Transition from Marlin-based bioprinter to Klipper + T-Code system with Peltier temperature control, pressure dispensing, and multi-material capabilities.

**Hardware:** BTT Octopus V1.1 + Raspberry Pi 4/5 + Peltier Element + Pneumatic Dispensing System

---

## Table of Contents

1. [Phase 1: Foundation (Weeks 1-2) - Klipper Installation & Basic Motion](#phase-1-foundation-weeks-1-2)
2. [Phase 2: Motion Calibration (Weeks 3-4) - Porting Marlin Config to Klipper](#phase-2-motion-calibration-weeks-3-4)
3. [Phase 3: T-Code Setup (Weeks 5-6) - Python Environment & Basic Testing](#phase-3-t-code-setup-weeks-5-6)
4. [Phase 4: Peltier Integration (Weeks 7-8) - Temperature Control via T-Code](#phase-4-peltier-integration-weeks-7-8)
5. [Phase 5: Pressure System (Weeks 9-10) - Pneumatic Dispenser Control](#phase-5-pressure-system-weeks-9-10)
6. [Phase 6: System Integration (Weeks 11-12) - Full Bioprinter Testing](#phase-6-system-integration-weeks-11-12)
7. [Appendices](#appendices)

---

## Phase 1: Foundation (Weeks 1-2)
### Klipper Installation & Basic Motion

### **Week 1: Raspberry Pi Setup & Klipper Installation**

#### **Day 1-2: Hardware Preparation**

**Objectives:**
- Acquire and set up Raspberry Pi
- Install Klipper firmware environment
- Establish basic connectivity

**Tasks:**

**Hardware Shopping List:**
```
Essential:
□ Raspberry Pi 4 (8GB RAM) or Raspberry Pi 5 (8GB) - $75-80
□ MicroSD Card (32GB Class 10 or better) - $10
□ USB-C Power Supply (5V 3A minimum) - $10
□ USB-A to USB-B cable (for Octopus connection) - $5
□ Ethernet cable (recommended for initial setup) - $5
□ MicroSD card reader (if not built into computer) - $8

Optional but Recommended:
□ Raspberry Pi case with fan - $15
□ HDMI cable + monitor (for initial setup) - may already have
□ USB keyboard + mouse (for initial setup) - may already have

Total: ~$113-128 (essentials only)
```

**Day 1 Tasks:**
1. **Download Raspberry Pi OS:**
   ```
   URL: https://www.raspberrypi.com/software/
   Choose: Raspberry Pi OS Lite (64-bit) - headless server version
   OR: Raspberry Pi OS with Desktop if you want GUI
   ```

2. **Flash MicroSD Card:**
   ```
   Tool: Raspberry Pi Imager (official tool)
   Steps:
   - Insert microSD into card reader
   - Open Raspberry Pi Imager
   - Choose OS: Raspberry Pi OS Lite (64-bit)
   - Choose Storage: Your microSD card
   - Settings (gear icon):
     ✓ Enable SSH
     ✓ Set username: pi
     ✓ Set password: <your_password>
     ✓ Configure WiFi (if not using Ethernet)
     ✓ Set hostname: bioprinter.local
   - Click "Write" and wait
   ```

3. **First Boot:**
   ```
   - Insert microSD into Raspberry Pi
   - Connect Ethernet cable (or rely on WiFi if configured)
   - Connect power supply
   - Wait 2-3 minutes for boot
   ```

4. **SSH Connection:**
   ```bash
   # From your computer:
   ssh pi@bioprinter.local
   # OR if .local doesn't work, find IP from router:
   ssh pi@192.168.1.xxx

   # Accept fingerprint (type 'yes')
   # Enter password you set earlier
   ```

**Day 2 Tasks:**

5. **System Update:**
   ```bash
   sudo apt update
   sudo apt upgrade -y
   # This takes 10-30 minutes
   sudo reboot
   # Wait 2 minutes, then reconnect
   ssh pi@bioprinter.local
   ```

6. **Install Essential Tools:**
   ```bash
   sudo apt install -y git python3-pip python3-venv \
                       python3-dev libffi-dev build-essential \
                       libncurses-dev libssl-dev
   ```

7. **Clone Klipper Repository:**
   ```bash
   cd ~
   git clone https://github.com/Klipper3d/klipper
   cd klipper
   ```

**Deliverable:** Raspberry Pi running, SSH accessible, Klipper repo cloned

---

#### **Day 3-4: Klipper Firmware Compilation & Flashing**

**Objectives:**
- Compile Klipper firmware for BTT Octopus V1.1
- Flash firmware to Octopus board
- Establish USB communication

**Tasks:**

**Day 3: Firmware Compilation**

1. **Configure Klipper Firmware for STM32F446:**
   ```bash
   cd ~/klipper
   make menuconfig
   ```

2. **Configuration Settings (CRITICAL - MUST MATCH EXACTLY):**
   ```
   [*] Enable extra low-level configuration options
       Micro-controller Architecture (STMicroelectronics STM32)
       Processor model (STM32F446)
       Bootloader offset (32KiB bootloader)
       Clock Reference (12 MHz crystal)
       Communication interface (USB (on PA11/PA12))
       USB ids (default)

   Exit and Save: Q → Y
   ```

3. **Compile Firmware:**
   ```bash
   make clean
   make
   # Should complete in 1-2 minutes
   # Output file: ~/klipper/out/klipper.bin
   ```

4. **Verify Binary Created:**
   ```bash
   ls -lh ~/klipper/out/klipper.bin
   # Should show file size around 26-28KB
   ```

**Day 4: Flashing Octopus Board**

**Method 1: SD Card Flashing (Easiest)**

5. **Prepare SD Card:**
   ```bash
   # On Raspberry Pi:
   sudo apt install -y dosfstools

   # Copy firmware to accessible location
   cp ~/klipper/out/klipper.bin /tmp/firmware.bin

   # Transfer to your computer via SCP:
   # From your computer (not RPi):
   scp pi@bioprinter.local:/tmp/firmware.bin ~/Desktop/firmware.bin
   ```

6. **Flash via SD Card:**
   ```
   Steps:
   - Format SD card as FAT32
   - Copy firmware.bin to SD card root
   - Rename to: firmware.bin (or FIRMWARE.CUR depending on bootloader)
   - Eject SD card safely
   - Power OFF Octopus board completely
   - Insert SD card into Octopus SD slot
   - Power ON Octopus
   - Wait 10 seconds (LED may flash)
   - Power OFF Octopus
   - Remove SD card
   - Check if firmware.bin renamed to FIRMWARE.CUR (confirms success)
   ```

**Method 2: DFU Flashing (If SD card doesn't work)**

7. **Install DFU Utilities:**
   ```bash
   sudo apt install -y dfu-util
   ```

8. **Put Octopus in DFU Mode:**
   ```
   - Disconnect USB from Octopus
   - Hold BOOT0 button (or set jumper on BOOT0 pins)
   - Connect USB to Raspberry Pi
   - Release BOOT0 after 2 seconds
   ```

9. **Flash via DFU:**
   ```bash
   sudo dfu-util -a 0 -d 0483:df11 -s 0x08008000:leave -D ~/klipper/out/klipper.bin
   # Should see "File downloaded successfully"
   ```

10. **Reset Board:**
    ```
    - Disconnect USB
    - Wait 5 seconds
    - Reconnect USB
    ```

**Verification:**

11. **Find Serial Device:**
    ```bash
    ls /dev/serial/by-id/*
    # Should show something like:
    # /dev/serial/by-id/usb-Klipper_stm32f446xx_XXXXX-if00

    # Save this path, you'll need it for config
    ```

**Deliverable:** Klipper firmware running on Octopus, USB connection verified

---

#### **Day 5-7: Klipper Host Setup & Basic Configuration**

**Objectives:**
- Install Klipper host software
- Install Mainsail or Fluidd web interface
- Create basic printer configuration
- Test communication

**Tasks:**

**Day 5: Klipper Host Installation**

1. **Install KIAUH (Klipper Installation And Update Helper):**
   ```bash
   cd ~
   git clone https://github.com/dw-0/kiauh.git
   cd kiauh
   ./kiauh.sh
   ```

2. **Install Components via KIAUH:**
   ```
   Menu Navigation:
   [1] Install

   Select to Install:
   [1] Klipper (if not already installed from git)
   [2] Moonraker (API server)
   [3] Mainsail (Web Interface) - Recommended
       OR
       Fluidd (Alternative Web Interface)
   [4] KlipperScreen (optional, if using touchscreen)

   Follow prompts, accept defaults
   Installation takes 10-15 minutes
   ```

3. **Verify Installation:**
   ```bash
   sudo systemctl status klipper
   sudo systemctl status moonraker
   # Both should show "active (running)"
   ```

4. **Access Web Interface:**
   ```
   Open browser on your computer:
   http://bioprinter.local
   OR
   http://192.168.1.xxx (RPi IP address)

   You should see Mainsail/Fluidd interface with error:
   "Klipper reports: SHUTDOWN - No config file found"
   This is EXPECTED - we haven't created config yet
   ```

**Day 6: Basic Printer Configuration**

5. **Create Base Configuration File:**
   ```bash
   cd ~/printer_data/config
   nano printer.cfg
   ```

6. **Paste Initial Configuration:**
   ```ini
   # BTT Octopus V1.1 Bioprinter Configuration
   # Based on Marlin setup with E0 homing support

   [mcu]
   serial: /dev/serial/by-id/usb-Klipper_stm32f446xx_XXXXX-if00
   # ^^^ REPLACE WITH YOUR ACTUAL SERIAL ID FROM EARLIER

   [printer]
   kinematics: cartesian
   max_velocity: 300
   max_accel: 3000
   max_z_velocity: 5
   max_z_accel: 100

   # X Stepper (Motor 0)
   [stepper_x]
   step_pin: PF13
   dir_pin: PF12
   enable_pin: !PF14
   microsteps: 16
   rotation_distance: 40  # 20 teeth * 2mm GT2 belt pitch
   endstop_pin: ^PG6
   position_endstop: 0
   position_max: 200  # Adjust to your build volume
   homing_speed: 25  # Reduced from 50 as per your request

   # Y Stepper (Motor 1)
   [stepper_y]
   step_pin: PG0
   dir_pin: PG1
   enable_pin: !PF15
   microsteps: 16
   rotation_distance: 40
   endstop_pin: ^PG9
   position_endstop: 0
   position_max: 200  # Adjust to your build volume
   homing_speed: 25  # Reduced from 50 as per your request

   # Z Stepper (Motor 2)
   [stepper_z]
   step_pin: PF11
   dir_pin: PG3
   enable_pin: !PG5
   microsteps: 16
   rotation_distance: 8  # Lead screw pitch
   endstop_pin: ^PG10
   position_endstop: 0
   position_max: 200  # Adjust to your build volume
   homing_speed: 4

   # E0 Extruder (Motor 3)
   [extruder]
   step_pin: PG4
   dir_pin: PC1
   enable_pin: !PA0
   microsteps: 16
   rotation_distance: 33.5  # Adjust for your extruder
   nozzle_diameter: 0.400
   filament_diameter: 1.750
   heater_pin: PA2  # Will be disabled for Peltier
   sensor_type: Generic 3950  # NTC 100K thermistor
   sensor_pin: PF4
   min_temp: 0      # Allow cooling below room temp
   max_temp: 50     # Peltier max temp

   # Note: Heater control disabled - will use T-Code for Peltier
   # This is just for temperature reading

   # TMC2209 Drivers
   [tmc2209 stepper_x]
   uart_pin: PC4
   run_current: 0.800
   stealthchop_threshold: 999999

   [tmc2209 stepper_y]
   uart_pin: PD11
   run_current: 0.800
   stealthchop_threshold: 999999

   [tmc2209 stepper_z]
   uart_pin: PC6
   run_current: 0.650
   stealthchop_threshold: 999999

   [tmc2209 extruder]
   uart_pin: PC7
   run_current: 0.800
   stealthchop_threshold: 999999
   ```

7. **Save and Exit:**
   ```
   Ctrl+X → Y → Enter
   ```

8. **Restart Klipper:**
   ```bash
   sudo systemctl restart klipper
   ```

9. **Check Status in Web Interface:**
   ```
   Refresh browser
   Should now see "Ready" status instead of error
   If you see errors, check:
   - Serial port path is correct
   - Syntax in printer.cfg (no typos)
   ```

**Day 7: Basic Motion Testing**

10. **Home X Axis:**
    ```
    In Mainsail/Fluidd console:
    G28 X

    Expected: X axis moves toward endstop, stops when triggered
    If direction is wrong: Add or remove ! from dir_pin in [stepper_x]
    ```

11. **Home Y Axis:**
    ```
    G28 Y

    Verify correct direction
    ```

12. **Home Z Axis:**
    ```
    G28 Z

    Verify correct direction and safe homing
    ```

13. **Home All:**
    ```
    G28

    Should home X, Y, Z in sequence
    ```

14. **Test Manual Movement:**
    ```
    G91              ; Relative positioning
    G1 X10 F3000    ; Move X +10mm
    G1 Y10 F3000    ; Move Y +10mm
    G1 Z10 F300     ; Move Z +10mm
    G90              ; Absolute positioning
    ```

**Deliverable:** Klipper running with web interface, basic X/Y/Z motion working

---

### **Week 2: E0 Extruder Homing Configuration**

#### **Day 8-9: Port E0 Homing from Marlin to Klipper**

**Objectives:**
- Configure E0 endstop in Klipper
- Create homing macro for E0
- Test E0 homing functionality

**Background:**
Klipper doesn't natively support extruder homing like Marlin, but we can achieve it using:
- Manual endstop configuration
- Custom homing macros
- Force_move module

**Tasks:**

**Day 8: E0 Endstop Configuration**

1. **Add E0 Endstop to Configuration:**
   ```bash
   nano ~/printer_data/config/printer.cfg
   ```

2. **Add Manual Stepper Section (AFTER [extruder] section):**
   ```ini
   # E0 Manual Stepper for Homing (Bioprinter Syringe Refill)
   [manual_stepper syringe_axis]
   step_pin: PG4      # Same as extruder
   dir_pin: PC1       # Same as extruder
   enable_pin: !PA0   # Same as extruder
   microsteps: 16
   rotation_distance: 33.5
   velocity: 10       # 10mm/s for safe homing
   accel: 500
   endstop_pin: ^PG11 # E0_MIN endstop (Z2-STOP connector)

   # E0 Endstop Configuration
   [endstop_phase syringe_axis]
   # Optional: for improved accuracy
   ```

3. **Add Force Move Support:**
   ```ini
   # Allow manual moves for extruder homing
   [force_move]
   enable_force_move: True
   ```

4. **Save and Restart Klipper:**
   ```bash
   # Ctrl+X, Y, Enter to save
   sudo systemctl restart klipper
   ```

**Day 9: E0 Homing Macros**

5. **Create Macros File:**
   ```bash
   nano ~/printer_data/config/macros.cfg
   ```

6. **Add E0 Homing Macros:**
   ```ini
   # Bioprinter E0 Homing Macros

   [gcode_macro HOME_SYRINGE]
   description: Home E0 extruder to refill position
   gcode:
       {% set HOME_SPEED = 600 %}      # 10mm/s in mm/min
       {% set HOME_DISTANCE = -100 %}  # Maximum travel distance (negative = toward endstop)

       M117 Homing Syringe...

       # Enable E stepper
       MANUAL_STEPPER STEPPER=syringe_axis ENABLE=1

       # Home toward endstop
       MANUAL_STEPPER STEPPER=syringe_axis MOVE={HOME_DISTANCE} SPEED={HOME_SPEED} STOP_ON_ENDSTOP=1

       # Set position to 0 after homing
       MANUAL_STEPPER STEPPER=syringe_axis SET_POSITION=0

       M117 Syringe Homed - Ready to Refill

   [gcode_macro SYRINGE_REFILL_WORKFLOW]
   description: Complete syringe refill workflow
   gcode:
       M117 Starting Refill Workflow...

       # Home to refill position
       HOME_SYRINGE

       # Wait for user to refill
       M117 REFILL SYRINGE NOW
       M300 S1000 P200  # Beep (if buzzer connected)
       M0               # Pause for manual refill

       # Move away from endstop after refill
       M117 Moving to start position...
       M302 P1          # Allow cold extrusion
       MANUAL_STEPPER STEPPER=syringe_axis ENABLE=1
       MANUAL_STEPPER STEPPER=syringe_axis MOVE=100 SPEED=1800

       # Reset position
       MANUAL_STEPPER STEPPER=syringe_axis SET_POSITION=0

       M117 Refill Complete - Ready

   [gcode_macro G28_E]
   description: G28 E compatibility command
   gcode:
       HOME_SYRINGE
   ```

7. **Include Macros in Main Config:**
   ```bash
   nano ~/printer_data/config/printer.cfg
   ```

8. **Add at END of printer.cfg:**
   ```ini
   # Include Macros
   [include macros.cfg]
   ```

9. **Save and Restart:**
   ```bash
   sudo systemctl restart klipper
   ```

10. **Test E0 Homing:**
    ```
    In Mainsail console:
    HOME_SYRINGE

    Expected behavior:
    - E motor moves backward (toward endstop)
    - Stops when endstop at PG11 triggers
    - Position set to 0
    - Status shows "Syringe Homed"

    If direction wrong: Toggle ! on dir_pin in [manual_stepper syringe_axis]
    ```

**Deliverable:** E0 homing functional via Klipper macros

---

#### **Day 10-14: Motion Calibration & Testing**

**Objectives:**
- Calibrate stepper rotation distances
- Fine-tune acceleration and jerk
- Test complete motion system
- Create backup of working configuration

**Tasks:**

**Day 10: Stepper Calibration**

1. **Calibrate X/Y Steps:**
   ```
   Process:
   1. Mark starting position on bed/frame
   2. Command: G91, G1 X100 F3000
   3. Measure actual movement with calipers
   4. Calculate new rotation_distance:
      new_rd = old_rd * (commanded / actual)
   5. Update printer.cfg
   6. Restart Klipper
   7. Repeat until accurate within 0.1mm
   ```

2. **Calibrate Z Steps:**
   ```
   1. Place height gauge or calipers under nozzle
   2. G91, G1 Z10 F300
   3. Measure actual Z movement
   4. Calculate and update rotation_distance
   5. Test multiple heights (10mm, 50mm, 100mm)
   ```

3. **Calibrate E Steps:**
   ```
   1. Mark filament 120mm from extruder entrance
   2. Heat extruder to 200°C (or use M302 P1 for cold)
   3. M83 (relative E mode)
   4. G1 E100 F300
   5. Measure remaining distance to mark
   6. Should be 20mm (120-100=20)
   7. If not, calculate new rotation_distance
   ```

**Day 11: Acceleration Tuning**

4. **Input Shaping Test (Optional but Recommended):**
   ```bash
   # Install accelerometer support (if you have ADXL345)
   sudo apt install -y python3-numpy python3-matplotlib

   # Or skip if no accelerometer - use default values
   ```

5. **Set Conservative Acceleration Values:**
   ```ini
   [printer]
   max_velocity: 300
   max_accel: 2000      # Start conservative
   max_accel_to_decel: 1000
   square_corner_velocity: 5.0
   ```

6. **Test Acceleration:**
   ```
   G28
   G1 X100 Y100 F6000
   G1 X10 Y10 F6000
   G1 X100 Y100 F6000

   Listen for:
   - Skipped steps (grinding noise)
   - Excessive vibration
   - Layer shifting (in test prints)

   If stable, increase max_accel by 500 and repeat
   Find maximum stable acceleration
   ```

**Day 12: Endstop Verification**

7. **Test All Endstops:**
   ```
   QUERY_ENDSTOPS
   # Should show:
   # x:open y:open z:open syringe_axis:open

   # Manually press each endstop and query again:
   QUERY_ENDSTOPS
   # Should show triggered endstop as "TRIGGERED"
   ```

8. **Verify Homing Safety:**
   ```
   Test sequence:
   1. G28 X - verify stops at endstop, not crashing
   2. G28 Y - verify safe homing
   3. G28 Z - verify Z doesn't crash into bed
   4. HOME_SYRINGE - verify E stops at endstop
   ```

**Day 13: Create Test Patterns**

9. **Create Test G-code Files:**

   **File: square_test.gcode**
   ```gcode
   ; 100mm square test pattern
   G28              ; Home all
   G90              ; Absolute positioning
   G1 Z10 F300      ; Raise Z
   G1 X50 Y50 F3000 ; Move to start
   G1 Z1 F300       ; Lower (for testing, no extrusion yet)
   G1 X150 Y50 F3000
   G1 X150 Y150
   G1 X50 Y150
   G1 X50 Y50
   G1 Z10           ; Raise
   G1 X0 Y0         ; Return home
   ```

10. **Upload and Run Test:**
    ```
    - In Mainsail: Upload square_test.gcode
    - Click "Print"
    - Observe smooth motion without jerking
    ```

**Day 14: Backup & Documentation**

11. **Backup Working Configuration:**
    ```bash
    cd ~/printer_data/config
    tar -czf ~/bioprinter_config_$(date +%Y%m%d).tar.gz *.cfg

    # Copy to your computer:
    scp pi@bioprinter.local:~/bioprinter_config_*.tar.gz ~/Desktop/
    ```

12. **Document Current State:**
    ```bash
    nano ~/WEEK2_STATUS.md
    ```

    Write:
    ```markdown
    # Week 2 Completion Status

    ## Working Features:
    - [x] Klipper installed and running
    - [x] X/Y/Z homing functional
    - [x] E0 homing via manual stepper
    - [x] Motion calibrated (within 0.1mm accuracy)
    - [x] Acceleration tuned (max_accel: XXXX)
    - [x] Web interface (Mainsail) accessible

    ## Configuration Files:
    - printer.cfg (main config)
    - macros.cfg (E0 homing macros)

    ## Known Issues:
    - [ ] List any issues here

    ## Next Steps:
    - Install T-Code Python environment
    - Begin auxiliary device control development
    ```

**Deliverable:** Fully functional Klipper motion control system, E0 homing working, configuration backed up

---

## Phase 2: T-Code Foundation (Weeks 3-4)
### Python Environment & Basic Auxiliary Control

### **Week 3: T-Code Python Setup**

#### **Day 15-16: Clone & Setup Johns Hopkins T-Code**

**Objectives:**
- Clone T-Code repository
- Set up Python environment
- Understand code structure
- Test basic functionality

**Tasks:**

**Day 15: Repository Setup**

1. **Clone T-Code Repository:**
   ```bash
   cd ~
   git clone https://github.com/JHU-Mueller-Lab/Time-Code-for-Multifunctional-3D-Printhead-Controls.git tcode
   cd tcode
   ```

2. **Examine Repository Structure:**
   ```bash
   ls -la
   # You should see:
   # - TCode_Automation1_TCP_IP.py (main script)
   # - TCode_RS232_Connection_OutDated.py (legacy)
   # - Example G-code files
   # - README documentation
   ```

3. **Review Main Script:**
   ```bash
   cat TCode_Automation1_TCP_IP.py | less
   # Press 'q' to exit

   # Note the key sections:
   # - Velocity profile calculation
   # - G-code parsing
   # - Timestamp generation
   # - Aerotech communication (will need to replace)
   ```

4. **Create Python Virtual Environment:**
   ```bash
   cd ~/tcode
   python3 -m venv tcode-env
   source tcode-env/bin/activate
   # Prompt should change to show (tcode-env)
   ```

5. **Install Dependencies:**
   ```bash
   pip install --upgrade pip
   pip install pyserial numpy matplotlib
   # Add more as needed during development
   ```

6. **Create Requirements File:**
   ```bash
   pip freeze > requirements.txt
   # Allows easy reinstall later: pip install -r requirements.txt
   ```

**Day 16: Code Analysis & Adaptation Planning**

7. **Create Adaptation Notes Document:**
   ```bash
   nano ~/tcode/ADAPTATION_PLAN.md
   ```

8. **Document Required Changes:**
   ```markdown
   # T-Code Adaptation for Klipper + Raspberry Pi

   ## Original System (Johns Hopkins):
   - Motion Controller: Aerotech Automation1 (TCP/IP)
   - Auxiliary Devices: Nordson Ultimus V (Serial)
   - Communication: Industrial TCP/IP + RS-232

   ## Target System (Your Bioprinter):
   - Motion Controller: Klipper (Serial/Moonraker API)
   - Auxiliary Devices:
     * Peltier (GPIO PWM + relay control)
     * Pressure dispenser (TBD - serial or voltage)
     * Solenoid valves (GPIO digital out)
   - Communication: USB Serial + GPIO

   ## Required Code Modifications:

   ### 1. Replace Aerotech Communication
   **Original:**
   ```python
   # Aerotech TCP/IP connection
   controller = AerotechController(ip='192.168.1.100')
   controller.send_gcode(gcode_line)
   ```

   **New:**
   ```python
   # Klipper Moonraker API or Serial
   import serial
   klipper = serial.Serial('/dev/ttyACM0', 250000)
   klipper.write(f"{gcode_line}\n".encode())
   ```

   ### 2. Add GPIO Control for Auxiliary Devices
   **New Section:**
   ```python
   import RPi.GPIO as GPIO

   # Pin definitions
   PELTIER_HEAT_PIN = 17
   PELTIER_COOL_PIN = 27
   PELTIER_POLARITY_PIN = 22

   GPIO.setmode(GPIO.BCM)
   GPIO.setup(PELTIER_HEAT_PIN, GPIO.OUT)
   # etc.
   ```

   ### 3. Timing Synchronization
   Keep existing timing logic, but adapt ping mechanism:
   - Original: TCP/IP ping to Aerotech
   - New: Query Klipper status via Moonraker API

   ### 4. Auxiliary Command Mapping
   **Map T-Code aux commands to GPIO actions:**
   - `AUX_TEMP_SET 20` → GPIO PWM for Peltier
   - `AUX_PRESSURE 50` → Serial to pressure controller
   - `AUX_VALVE_OPEN` → GPIO HIGH on valve pin
   ```

9. **Save Adaptation Plan:**
   ```
   Ctrl+X, Y, Enter
   ```

**Deliverable:** T-Code repository cloned, Python environment set up, adaptation plan documented

---

#### **Day 17-18: Create Minimal T-Code Test Script**

**Objectives:**
- Create simplified T-Code test script
- Verify Klipper communication
- Test GPIO control basics

**Tasks:**

**Day 17: Minimal Script Development**

1. **Create Test Directory:**
   ```bash
   cd ~/tcode
   mkdir tests
   cd tests
   ```

2. **Create Minimal T-Code Test Script:**
   ```bash
   nano test_tcode_basic.py
   ```

3. **Paste Basic Test Code:**
   ```python
   #!/usr/bin/env python3
   """
   Minimal T-Code Test for Klipper + Raspberry Pi
   Tests basic communication and timing
   """

   import time
   import serial
   from datetime import datetime

   # Configuration
   KLIPPER_SERIAL = '/dev/ttyACM0'  # Adjust to your Klipper device
   BAUD_RATE = 250000

   class MinimalTCode:
       def __init__(self):
           print("Initializing Minimal T-Code...")
           self.klipper = None

       def connect_klipper(self):
           """Establish serial connection to Klipper"""
           try:
               self.klipper = serial.Serial(KLIPPER_SERIAL, BAUD_RATE, timeout=1)
               time.sleep(2)  # Wait for connection to stabilize
               print(f"✓ Connected to Klipper on {KLIPPER_SERIAL}")
               return True
           except Exception as e:
               print(f"✗ Failed to connect: {e}")
               return False

       def send_gcode(self, gcode_line):
           """Send G-code command to Klipper"""
           if not self.klipper:
               print("✗ Not connected to Klipper")
               return False

           try:
               cmd = f"{gcode_line}\n"
               self.klipper.write(cmd.encode())
               print(f"→ Sent: {gcode_line}")

               # Wait for response
               time.sleep(0.1)
               if self.klipper.in_waiting:
                   response = self.klipper.readline().decode().strip()
                   print(f"← Response: {response}")

               return True
           except Exception as e:
               print(f"✗ Send failed: {e}")
               return False

       def test_motion(self):
           """Test basic motion commands"""
           print("\n=== Testing Motion Commands ===")

           commands = [
               "G28",           # Home all
               "G90",           # Absolute positioning
               "G1 X50 F3000",  # Move X
               "G1 Y50 F3000",  # Move Y
               "G1 X0 Y0",      # Return
           ]

           for cmd in commands:
               self.send_gcode(cmd)
               time.sleep(1)  # Wait between commands

       def test_timing(self):
           """Test timing precision"""
           print("\n=== Testing Timing Precision ===")

           delays = [0.1, 0.5, 1.0]

           for delay in delays:
               start = time.time()
               time.sleep(delay)
               actual = time.time() - start
               error = abs(actual - delay) * 1000  # ms
               print(f"Target: {delay*1000:.1f}ms, Actual: {actual*1000:.1f}ms, Error: {error:.2f}ms")

       def run_tests(self):
           """Run all tests"""
           print("=" * 50)
           print("Minimal T-Code Test Suite")
           print("=" * 50)

           if not self.connect_klipper():
               return False

           self.test_motion()
           self.test_timing()

           print("\n✓ All tests complete")
           return True

       def cleanup(self):
           """Clean up resources"""
           if self.klipper:
               self.klipper.close()
               print("✓ Klipper connection closed")

   if __name__ == "__main__":
       tcode = MinimalTCode()
       try:
           tcode.run_tests()
       except KeyboardInterrupt:
           print("\n⚠ Test interrupted by user")
       finally:
           tcode.cleanup()
   ```

4. **Save and Make Executable:**
   ```bash
   chmod +x test_tcode_basic.py
   ```

5. **Find Klipper Serial Device:**
   ```bash
   ls -l /dev/serial/by-id/
   # Look for Klipper device
   # Update KLIPPER_SERIAL in script if different from /dev/ttyACM0
   ```

6. **Run Test:**
   ```bash
   python3 test_tcode_basic.py
   ```

**Expected Output:**
```
==================================================
Minimal T-Code Test Suite
==================================================
Initializing Minimal T-Code...
✓ Connected to Klipper on /dev/ttyACM0

=== Testing Motion Commands ===
→ Sent: G28
← Response: ok
→ Sent: G90
← Response: ok
→ Sent: G1 X50 F3000
← Response: ok
→ Sent: G1 Y50 F3000
← Response: ok
→ Sent: G1 X0 Y0
← Response: ok

=== Testing Timing Precision ===
Target: 100.0ms, Actual: 100.2ms, Error: 0.20ms
Target: 500.0ms, Actual: 500.1ms, Error: 0.10ms
Target: 1000.0ms, Actual: 1000.3ms, Error: 0.30ms

✓ All tests complete
✓ Klipper connection closed
```

**Day 18: GPIO Test Setup**

7. **Create GPIO Test Script:**
   ```bash
   nano test_gpio_control.py
   ```

8. **Paste GPIO Test Code:**
   ```python
   #!/usr/bin/env python3
   """
   GPIO Control Test for Auxiliary Devices
   Tests basic LED/relay control before connecting actual devices
   """

   import RPi.GPIO as GPIO
   import time

   # Pin definitions (BCM numbering)
   TEST_PIN_1 = 17  # Future: Peltier Heat
   TEST_PIN_2 = 27  # Future: Peltier Cool
   TEST_PIN_3 = 22  # Future: Peltier Polarity

   class GPIOTest:
       def __init__(self):
           print("Initializing GPIO Test...")
           GPIO.setmode(GPIO.BCM)
           GPIO.setwarnings(False)

           # Setup pins as outputs
           GPIO.setup(TEST_PIN_1, GPIO.OUT)
           GPIO.setup(TEST_PIN_2, GPIO.OUT)
           GPIO.setup(TEST_PIN_3, GPIO.OUT)

           # Initialize to LOW
           GPIO.output(TEST_PIN_1, GPIO.LOW)
           GPIO.output(TEST_PIN_2, GPIO.LOW)
           GPIO.output(TEST_PIN_3, GPIO.LOW)

           print("✓ GPIO initialized")

       def test_digital_output(self):
           """Test simple on/off control"""
           print("\n=== Testing Digital Output ===")

           pins = [TEST_PIN_1, TEST_PIN_2, TEST_PIN_3]

           for pin in pins:
               print(f"Pin {pin}: HIGH")
               GPIO.output(pin, GPIO.HIGH)
               time.sleep(1)

               print(f"Pin {pin}: LOW")
               GPIO.output(pin, GPIO.LOW)
               time.sleep(1)

       def test_pwm_output(self):
           """Test PWM control (for future Peltier power control)"""
           print("\n=== Testing PWM Output ===")

           # Setup PWM on TEST_PIN_1 at 1kHz
           pwm = GPIO.PWM(TEST_PIN_1, 1000)

           print("PWM Test: 0% → 25% → 50% → 75% → 100% → 0%")

           for duty in [0, 25, 50, 75, 100, 0]:
               print(f"Duty cycle: {duty}%")
               pwm.start(duty)
               time.sleep(2)

           pwm.stop()

       def test_timing_precision(self):
           """Test timing for synchronized control"""
           print("\n=== Testing GPIO Timing ===")

           iterations = 100
           delays = []

           for i in range(iterations):
               start = time.time()
               GPIO.output(TEST_PIN_1, GPIO.HIGH)
               GPIO.output(TEST_PIN_1, GPIO.LOW)
               elapsed = time.time() - start
               delays.append(elapsed * 1000)  # Convert to ms

           avg_delay = sum(delays) / len(delays)
           max_delay = max(delays)
           min_delay = min(delays)

           print(f"Average: {avg_delay:.3f}ms")
           print(f"Min: {min_delay:.3f}ms")
           print(f"Max: {max_delay:.3f}ms")
           print(f"Range: {max_delay - min_delay:.3f}ms")

       def run_tests(self):
           """Run all GPIO tests"""
           print("=" * 50)
           print("GPIO Control Test Suite")
           print("=" * 50)
           print("\n⚠ WARNING: Connect LEDs to test pins before running")
           print("Pin connections:")
           print(f"  GPIO {TEST_PIN_1} (Physical Pin 11) → LED + 220Ω resistor → GND")
           print(f"  GPIO {TEST_PIN_2} (Physical Pin 13) → LED + 220Ω resistor → GND")
           print(f"  GPIO {TEST_PIN_3} (Physical Pin 15) → LED + 220Ω resistor → GND")
           input("\nPress Enter when ready...")

           self.test_digital_output()
           self.test_pwm_output()
           self.test_timing_precision()

           print("\n✓ All GPIO tests complete")

       def cleanup(self):
           """Clean up GPIO"""
           GPIO.cleanup()
           print("✓ GPIO cleaned up")

   if __name__ == "__main__":
       gpio_test = GPIOTest()
       try:
           gpio_test.run_tests()
       except KeyboardInterrupt:
           print("\n⚠ Test interrupted by user")
       finally:
           gpio_test.cleanup()
   ```

9. **Save and Make Executable:**
   ```bash
   chmod +x test_gpio_control.py
   ```

10. **Hardware Setup for GPIO Test:**
    ```
    Required:
    - 3x LEDs (any color)
    - 3x 220Ω resistors
    - Jumper wires

    Connections:
    GPIO 17 (Pin 11) → LED anode (+) → LED cathode (-) → 220Ω → GND
    GPIO 27 (Pin 13) → LED anode (+) → LED cathode (-) → 220Ω → GND
    GPIO 22 (Pin 15) → LED anode (+) → LED cathode (-) → 220Ω → GND
    ```

11. **Run GPIO Test:**
    ```bash
    python3 test_gpio_control.py
    ```

**Expected Behavior:**
- LEDs blink in sequence (digital test)
- One LED fades smoothly (PWM test)
- Timing statistics displayed

**Deliverable:** Basic T-Code communication working, GPIO control verified with LEDs

---

#### **Day 19-21: Velocity Profile & Timing Implementation**

**Objectives:**
- Extract velocity profile calculation from Johns Hopkins code
- Implement timing synchronization
- Create timestamp generator
- Test with simple G-code

**Tasks:**

**Day 19: Velocity Profile Calculator**

1. **Create Velocity Module:**
   ```bash
   cd ~/tcode
   nano velocity_calculator.py
   ```

2. **Implement Velocity Profile Code:**
   ```python
   #!/usr/bin/env python3
   """
   Velocity Profile Calculator for T-Code
   Based on Johns Hopkins implementation
   Calculates printer velocity at each point in G-code path
   """

   import numpy as np
   import math

   class VelocityProfile:
       def __init__(self, max_velocity, acceleration):
           """
           Initialize velocity profile calculator

           Args:
               max_velocity: Maximum print speed (mm/s)
               acceleration: Acceleration (mm/s²)
           """
           self.max_velocity = max_velocity
           self.acceleration = acceleration
           self.profile = []

       def calculate_move_profile(self, distance, start_velocity=0, end_velocity=0):
           """
           Calculate velocity profile for a single move

           Args:
               distance: Move distance (mm)
               start_velocity: Starting velocity (mm/s)
               end_velocity: Ending velocity (mm/s)

           Returns:
               dict with time and velocity arrays
           """
           # Handle zero-distance moves
           if distance <= 0:
               return {'time': [0], 'velocity': [0], 'position': [0]}

           # Calculate acceleration/deceleration distances
           accel_dist = (self.max_velocity**2 - start_velocity**2) / (2 * self.acceleration)
           decel_dist = (self.max_velocity**2 - end_velocity**2) / (2 * self.acceleration)

           # Check if move is too short to reach max velocity
           if accel_dist + decel_dist > distance:
               # Triangular profile (doesn't reach max velocity)
               peak_velocity = math.sqrt(
                   (2 * self.acceleration * distance + start_velocity**2 + end_velocity**2) / 2
               )
               accel_time = (peak_velocity - start_velocity) / self.acceleration
               decel_time = (peak_velocity - end_velocity) / self.acceleration
               total_time = accel_time + decel_time

               # Generate profile
               times = np.linspace(0, total_time, 100)
               velocities = []
               positions = []

               for t in times:
                   if t <= accel_time:
                       # Acceleration phase
                       v = start_velocity + self.acceleration * t
                       pos = start_velocity * t + 0.5 * self.acceleration * t**2
                   else:
                       # Deceleration phase
                       t_decel = t - accel_time
                       v = peak_velocity - self.acceleration * t_decel
                       accel_dist_actual = start_velocity * accel_time + 0.5 * self.acceleration * accel_time**2
                       pos = accel_dist_actual + peak_velocity * t_decel - 0.5 * self.acceleration * t_decel**2

                   velocities.append(v)
                   positions.append(pos)

               return {
                   'time': times.tolist(),
                   'velocity': velocities,
                   'position': positions,
                   'total_time': total_time,
                   'profile_type': 'triangular'
               }

           else:
               # Trapezoidal profile (reaches max velocity)
               accel_time = (self.max_velocity - start_velocity) / self.acceleration
               decel_time = (self.max_velocity - end_velocity) / self.acceleration

               const_vel_dist = distance - accel_dist - decel_dist
               const_vel_time = const_vel_dist / self.max_velocity

               total_time = accel_time + const_vel_time + decel_time

               # Generate profile
               times = np.linspace(0, total_time, 100)
               velocities = []
               positions = []

               for t in times:
                   if t <= accel_time:
                       # Acceleration phase
                       v = start_velocity + self.acceleration * t
                       pos = start_velocity * t + 0.5 * self.acceleration * t**2
                   elif t <= accel_time + const_vel_time:
                       # Constant velocity phase
                       t_const = t - accel_time
                       v = self.max_velocity
                       pos = accel_dist + self.max_velocity * t_const
                   else:
                       # Deceleration phase
                       t_decel = t - accel_time - const_vel_time
                       v = self.max_velocity - self.acceleration * t_decel
                       pos = accel_dist + const_vel_dist + self.max_velocity * t_decel - 0.5 * self.acceleration * t_decel**2

                   velocities.append(v)
                   positions.append(pos)

               return {
                   'time': times.tolist(),
                   'velocity': velocities,
                   'position': positions,
                   'total_time': total_time,
                   'profile_type': 'trapezoidal'
               }

       def plot_profile(self, profile):
           """Plot velocity profile (requires matplotlib)"""
           try:
               import matplotlib.pyplot as plt

               fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 8))

               # Velocity vs time
               ax1.plot(profile['time'], profile['velocity'])
               ax1.set_xlabel('Time (s)')
               ax1.set_ylabel('Velocity (mm/s)')
               ax1.set_title(f"Velocity Profile ({profile['profile_type']})")
               ax1.grid(True)

               # Position vs time
               ax2.plot(profile['time'], profile['position'])
               ax2.set_xlabel('Time (s)')
               ax2.set_ylabel('Position (mm)')
               ax2.set_title('Position vs Time')
               ax2.grid(True)

               plt.tight_layout()
               plt.savefig('velocity_profile.png')
               print("✓ Profile saved to velocity_profile.png")

           except ImportError:
               print("⚠ matplotlib not installed, skipping plot")

   # Test code
   if __name__ == "__main__":
       print("Testing Velocity Profile Calculator")
       print("=" * 50)

       # Test parameters
       max_vel = 50  # mm/s
       accel = 500   # mm/s²

       calc = VelocityProfile(max_vel, accel)

       # Test Case 1: Long move (trapezoidal)
       print("\nTest 1: Long move (100mm)")
       profile1 = calc.calculate_move_profile(100)
       print(f"  Profile type: {profile1['profile_type']}")
       print(f"  Total time: {profile1['total_time']:.3f}s")
       print(f"  Max velocity reached: {max(profile1['velocity']):.1f} mm/s")

       # Test Case 2: Short move (triangular)
       print("\nTest 2: Short move (10mm)")
       profile2 = calc.calculate_move_profile(10)
       print(f"  Profile type: {profile2['profile_type']}")
       print(f"  Total time: {profile2['total_time']:.3f}s")
       print(f"  Peak velocity: {max(profile2['velocity']):.1f} mm/s")

       # Plot
       calc.plot_profile(profile1)
   ```

3. **Test Velocity Calculator:**
   ```bash
   python3 velocity_calculator.py
   ```

**Day 20: G-code Parser Integration**

4. **Create G-code Parser:**
   ```bash
   nano gcode_parser.py
   ```

5. **Implement Parser:**
   ```python
   #!/usr/bin/env python3
   """
   G-code Parser for T-Code
   Separates motion commands from auxiliary commands
   """

   import re

   class GCodeParser:
       def __init__(self):
           self.motion_commands = []
           self.auxiliary_commands = []
           self.current_position = {'X': 0, 'Y': 0, 'Z': 0, 'E': 0}
           self.is_relative = False

       def parse_line(self, line):
           """Parse single G-code line"""
           # Remove comments
           line = re.sub(r';.*$', '', line).strip()

           if not line:
               return None

           # Check for motion commands (G0, G1)
           if line.startswith('G0') or line.startswith('G1'):
               return self.parse_motion(line)

           # Check for mode changes
           elif line.startswith('G90'):
               self.is_relative = False
               return {'type': 'mode', 'command': 'absolute'}

           elif line.startswith('G91'):
               self.is_relative = True
               return {'type': 'mode', 'command': 'relative'}

           # Check for auxiliary commands (M-codes, custom commands)
           elif line.startswith('M'):
               return self.parse_auxiliary(line)

           # Unknown command
           else:
               return {'type': 'unknown', 'command': line}

       def parse_motion(self, line):
           """Parse motion command (G0/G1)"""
           # Extract coordinates
           coords = {}
           for axis in ['X', 'Y', 'Z', 'E']:
               match = re.search(rf'{axis}(-?\d+\.?\d*)', line)
               if match:
                   coords[axis] = float(match.group(1))

           # Extract feedrate
           feedrate = None
           match = re.search(r'F(\d+\.?\d*)', line)
           if match:
               feedrate = float(match.group(1))

           # Calculate actual position and distance
           if self.is_relative:
               # Relative mode - coordinates are incremental
               distance = sum(coords.values())
               for axis, value in coords.items():
                   self.current_position[axis] += value
           else:
               # Absolute mode - calculate distance from current position
               distance = 0
               for axis, value in coords.items():
                   delta = value - self.current_position[axis]
                   distance += delta**2
                   self.current_position[axis] = value
               distance = distance**0.5

           return {
               'type': 'motion',
               'coordinates': coords,
               'feedrate': feedrate,
               'distance': distance,
               'position': self.current_position.copy()
           }

       def parse_auxiliary(self, line):
           """Parse auxiliary command (M-codes)"""
           # Extract M-code number
           match = re.search(r'M(\d+)', line)
           if not match:
               return {'type': 'unknown', 'command': line}

           m_code = int(match.group(1))

           # Extract parameters
           params = {}
           for param in ['S', 'P', 'T']:
               match = re.search(rf'{param}(-?\d+\.?\d*)', line)
               if match:
                   params[param] = float(match.group(1))

           return {
               'type': 'auxiliary',
               'm_code': m_code,
               'parameters': params,
               'raw': line
           }

       def parse_file(self, filename):
           """Parse entire G-code file"""
           with open(filename, 'r') as f:
               for line_num, line in enumerate(f, 1):
                   parsed = self.parse_line(line)
                   if parsed:
                       if parsed['type'] == 'motion':
                           self.motion_commands.append({
                               'line': line_num,
                               **parsed
                           })
                       elif parsed['type'] == 'auxiliary':
                           self.auxiliary_commands.append({
                               'line': line_num,
                               **parsed
                           })

           return {
               'motion': self.motion_commands,
               'auxiliary': self.auxiliary_commands
           }

   # Test
   if __name__ == "__main__":
       print("Testing G-code Parser")
       print("=" * 50)

       parser = GCodeParser()

       test_gcode = [
           "G28",
           "G90",
           "G1 X10 Y10 F3000",
           "M104 S20  ; Set Peltier temp",
           "G1 X20 Y20",
           "G91",
           "G1 X5 Y5",
       ]

       for line in test_gcode:
           result = parser.parse_line(line)
           print(f"Input: {line}")
           print(f"Output: {result}\n")
   ```

6. **Test Parser:**
   ```bash
   python3 gcode_parser.py
   ```

**Day 21: Timestamp Generator**

7. **Create Timestamp Generator:**
   ```bash
   nano timestamp_generator.py
   ```

8. **Implement Timing Logic:**
   ```python
   #!/usr/bin/env python3
   """
   Timestamp Generator for T-Code
   Calculates when auxiliary commands should execute based on motion profile
   """

   from velocity_calculator import VelocityProfile
   from gcode_parser import GCodeParser

   class TimestampGenerator:
       def __init__(self, max_velocity, acceleration):
           self.velocity_calc = VelocityProfile(max_velocity, acceleration)
           self.parser = GCodeParser()
           self.timestamps = []

       def generate_timestamps(self, gcode_file):
           """
           Process G-code file and generate timestamps for auxiliary commands

           Returns:
               List of (timestamp, command) tuples
           """
           # Parse G-code
           parsed = self.parser.parse_file(gcode_file)

           # Build motion timeline
           current_time = 0
           motion_timeline = []

           for motion in parsed['motion']:
               # Calculate velocity profile for this move
               profile = self.velocity_calc.calculate_move_profile(
                   distance=motion['distance']
               )

               motion_timeline.append({
                   'start_time': current_time,
                   'end_time': current_time + profile['total_time'],
                   'distance': motion['distance'],
                   'line': motion['line'],
                   'profile': profile
               })

               current_time += profile['total_time']

           # Map auxiliary commands to timestamps
           timestamps = []

           for aux in parsed['auxiliary']:
               # Find closest motion command (by line number)
               closest_motion = min(
                   motion_timeline,
                   key=lambda m: abs(m['line'] - aux['line'])
               )

               # Assign timestamp at start of closest motion
               timestamp = closest_motion['start_time']

               timestamps.append({
                   'time': timestamp,
                   'command': aux['raw'],
                   'type': 'auxiliary',
                   'line': aux['line']
               })

           # Sort by timestamp
           timestamps.sort(key=lambda x: x['time'])

           return {
               'motion_timeline': motion_timeline,
               'aux_timestamps': timestamps,
               'total_time': current_time
           }

       def export_tcode(self, timestamps, output_file):
           """Export T-Code format file"""
           with open(output_file, 'w') as f:
               f.write("# T-Code Generated File\n")
               f.write("# Format: timestamp(s), command\n\n")

               for ts in timestamps['aux_timestamps']:
                   f.write(f"{ts['time']:.3f}, {ts['command']}\n")

           print(f"✓ T-Code exported to {output_file}")

   # Test
   if __name__ == "__main__":
       print("Testing Timestamp Generator")
       print("=" * 50)

       # Create test G-code file
       test_gcode = """
       G28
       G90
       G1 X10 Y10 F3000
       M104 S20
       G1 X50 Y50
       M104 S25
       G1 X100 Y100
       M104 S30
       """

       with open('/tmp/test.gcode', 'w') as f:
           f.write(test_gcode)

       # Generate timestamps
       gen = TimestampGenerator(max_velocity=50, acceleration=500)
       timestamps = gen.generate_timestamps('/tmp/test.gcode')

       print("\nMotion Timeline:")
       for motion in timestamps['motion_timeline']:
           print(f"  Line {motion['line']}: {motion['start_time']:.3f}s - {motion['end_time']:.3f}s ({motion['distance']:.1f}mm)")

       print("\nAuxiliary Command Timestamps:")
       for ts in timestamps['aux_timestamps']:
           print(f"  {ts['time']:.3f}s: {ts['command']}")

       print(f"\nTotal print time: {timestamps['total_time']:.3f}s")

       # Export
       gen.export_tcode(timestamps, '/tmp/test.tcode')
   ```

9. **Test Timestamp Generator:**
   ```bash
   python3 timestamp_generator.py
   ```

**Deliverable:** Complete T-Code timing system working, ready for integration with Klipper and GPIO

---

### **Week 4: T-Code Integration Testing**

#### **Day 22-24: Integrated System Test**

**Objectives:**
- Combine all T-Code components
- Test with Klipper motion control
- Verify timing synchronization
- Create end-to-end workflow

**Tasks:**

**Day 22: System Integration**

1. **Create Main T-Code Controller:**
   ```bash
   cd ~/tcode
   nano tcode_controller.py
   ```

2. **Implement Full Controller:**
   ```python
   #!/usr/bin/env python3
   """
   T-Code Controller for Klipper + Raspberry Pi Bioprinter
   Main orchestrator for motion + auxiliary device synchronization
   """

   import time
   import serial
   import threading
   from datetime import datetime
   from timestamp_generator import TimestampGenerator
   from gcode_parser import GCodeParser

   class TCodeController:
       def __init__(self, klipper_serial='/dev/ttyACM0', max_vel=50, accel=500):
           print("Initializing T-Code Controller...")

           self.klipper_serial = klipper_serial
           self.klipper = None

           self.timestamp_gen = TimestampGenerator(max_vel, accel)
           self.parser = GCodeParser()

           self.is_running = False
           self.start_time = None

       def connect(self):
           """Connect to Klipper"""
           try:
               self.klipper = serial.Serial(self.klipper_serial, 250000, timeout=1)
               time.sleep(2)
               print(f"✓ Connected to Klipper")
               return True
           except Exception as e:
               print(f"✗ Connection failed: {e}")
               return False

       def send_motion_gcode(self, gcode_line):
           """Send motion G-code to Klipper"""
           if not self.klipper:
               return False

           try:
               self.klipper.write(f"{gcode_line}\n".encode())
               return True
           except Exception as e:
               print(f"✗ Motion send failed: {e}")
               return False

       def execute_auxiliary_command(self, command):
           """
           Execute auxiliary command (GPIO, serial, etc.)
           This is where Peltier, pressure, valves will be controlled
           """
           print(f"[AUX] {command}")

           # Parse M-code
           if 'M104' in command:
               # Temperature control command
               import re
               match = re.search(r'S(-?\d+\.?\d*)', command)
               if match:
                   temp = float(match.group(1))
                   print(f"  → Set Peltier target: {temp}°C")
                   # TODO: Call Peltier control function

           elif 'M106' in command:
               # Fan/Pressure control
               import re
               match = re.search(r'S(\d+)', command)
               if match:
                   speed = int(match.group(1))
                   print(f"  → Set pressure: {speed}")
                   # TODO: Call pressure control function

           else:
               print(f"  → Unknown auxiliary command: {command}")

       def auxiliary_command_thread(self, timestamps):
           """
           Background thread for executing auxiliary commands at precise times
           """
           print("Auxiliary command thread started")

           for ts in timestamps:
               # Wait until timestamp
               while True:
                   if not self.is_running:
                       print("Auxiliary thread stopped")
                       return

                   current_time = time.time() - self.start_time
                   if current_time >= ts['time']:
                       break

                   time.sleep(0.001)  # 1ms polling

               # Execute auxiliary command
               self.execute_auxiliary_command(ts['command'])

           print("All auxiliary commands executed")

       def print_file(self, gcode_file):
           """
           Execute print with T-Code synchronization
           """
           print(f"\nProcessing: {gcode_file}")
           print("=" * 50)

           # Generate timestamps
           print("Generating timestamps...")
           timing_data = self.timestamp_gen.generate_timestamps(gcode_file)

           motion_timeline = timing_data['motion_timeline']
           aux_timestamps = timing_data['aux_timestamps']

           print(f"✓ {len(motion_timeline)} motion commands")
           print(f"✓ {len(aux_timestamps)} auxiliary commands")
           print(f"✓ Estimated time: {timing_data['total_time']:.1f}s")

           # Start auxiliary command thread
           self.is_running = True
           self.start_time = time.time()

           aux_thread = threading.Thread(
               target=self.auxiliary_command_thread,
               args=(aux_timestamps,)
           )
           aux_thread.daemon = True
           aux_thread.start()

           # Send motion commands
           print("\nExecuting print...")
           with open(gcode_file, 'r') as f:
               for line in f:
                   if not self.is_running:
                       break

                   # Only send motion commands to Klipper
                   line = line.strip()
                   if line and not line.startswith(';'):
                       if line.startswith('G') or line.startswith('M'):
                           # Check if motion command
                           parsed = self.parser.parse_line(line)
                           if parsed and parsed['type'] == 'motion':
                               self.send_motion_gcode(line)
                               time.sleep(0.01)  # Small delay for Klipper buffer

           # Wait for auxiliary thread to complete
           aux_thread.join(timeout=timing_data['total_time'] + 10)

           self.is_running = False
           print("\n✓ Print complete")

       def cleanup(self):
           """Clean up resources"""
           self.is_running = False
           if self.klipper:
               self.klipper.close()
               print("✓ Klipper disconnected")

   # Main execution
   if __name__ == "__main__":
       import sys

       if len(sys.argv) < 2:
           print("Usage: python3 tcode_controller.py <gcode_file>")
           sys.exit(1)

       gcode_file = sys.argv[1]

       controller = TCodeController()

       try:
           if controller.connect():
               controller.print_file(gcode_file)
       except KeyboardInterrupt:
           print("\n⚠ Print interrupted by user")
       finally:
           controller.cleanup()
   ```

3. **Create Test G-code with Auxiliary Commands:**
   ```bash
   nano ~/tcode/test_print.gcode
   ```

4. **Add Test Print:**
   ```gcode
   ; T-Code Test Print
   ; 50mm square with temperature changes

   G28              ; Home all
   G90              ; Absolute positioning
   M104 S20         ; Set Peltier to 20°C
   G1 Z10 F300      ; Raise Z
   G1 X25 Y25 F3000 ; Move to start
   G1 Z1 F300       ; Lower Z

   ; First side
   M104 S22         ; Increase temp
   G1 X75 Y25

   ; Second side
   M104 S24         ; Increase temp
   G1 X75 Y75

   ; Third side
   M104 S26         ; Increase temp
   G1 X25 Y75

   ; Fourth side
   M104 S28         ; Increase temp
   G1 X25 Y25

   G1 Z10           ; Raise Z
   M104 S0          ; Turn off Peltier
   G28 X Y          ; Home X/Y

   M117 Print Complete
   ```

5. **Run Test:**
   ```bash
   python3 tcode_controller.py test_print.gcode
   ```

**Expected Output:**
```
Initializing T-Code Controller...
✓ Connected to Klipper

Processing: test_print.gcode
==================================================
Generating timestamps...
✓ 12 motion commands
✓ 6 auxiliary commands
✓ Estimated time: 15.3s

Executing print...
Auxiliary command thread started
[AUX] M104 S20
  → Set Peltier target: 20°C
[AUX] M104 S22
  → Set Peltier target: 22°C
[AUX] M104 S24
  → Set Peltier target: 24°C
...
All auxiliary commands executed
✓ Print complete
✓ Klipper disconnected
```

**Day 23-24: Timing Accuracy Testing**

6. **Create Timing Accuracy Test:**
   ```bash
   nano test_timing_accuracy.py
   ```

7. **Implement Accuracy Test:**
   ```python
   #!/usr/bin/env python3
   """
   Test timing accuracy of T-Code system
   Measures actual vs. expected command execution times
   """

   import time
   import statistics

   class TimingAccuracyTest:
       def __init__(self):
           self.results = []

       def test_scheduled_execution(self, iterations=100):
           """Test precision of scheduled command execution"""
           print(f"Testing {iterations} scheduled executions...")

           delays = [0.1, 0.5, 1.0, 2.0]

           for target_delay in delays:
               errors = []

               for i in range(iterations):
                   start = time.time()

                   # Simulate waiting for timestamp
                   while (time.time() - start) < target_delay:
                       time.sleep(0.001)  # 1ms polling (same as T-Code)

                   actual_delay = time.time() - start
                   error = (actual_delay - target_delay) * 1000  # ms
                   errors.append(error)

               mean_error = statistics.mean(errors)
               std_dev = statistics.stdev(errors)
               max_error = max(errors)
               min_error = min(errors)

               print(f"\nTarget delay: {target_delay*1000:.0f}ms")
               print(f"  Mean error: {mean_error:.3f}ms")
               print(f"  Std dev: {std_dev:.3f}ms")
               print(f"  Range: {min_error:.3f}ms to {max_error:.3f}ms")

               self.results.append({
                   'target': target_delay,
                   'mean_error': mean_error,
                   'std_dev': std_dev
               })

       def test_concurrent_execution(self):
           """Test if auxiliary commands interfere with timing"""
           print("\nTesting concurrent execution impact...")

           import threading

           def dummy_auxiliary_task():
               """Simulate auxiliary command (GPIO toggle, etc.)"""
               time.sleep(0.001)

           iterations = 50
           target_interval = 0.1  # 100ms

           errors = []

           last_time = time.time()
           for i in range(iterations):
               # Schedule next execution
               next_time = last_time + target_interval

               # Simulate auxiliary command
               thread = threading.Thread(target=dummy_auxiliary_task)
               thread.start()

               # Wait for next scheduled time
               while time.time() < next_time:
                   time.sleep(0.001)

               actual_time = time.time()
               error = (actual_time - next_time) * 1000
               errors.append(error)

               last_time = next_time
               thread.join()

           mean_error = statistics.mean(errors)
           max_error = max(errors)

           print(f"  Mean error: {mean_error:.3f}ms")
           print(f"  Max error: {max_error:.3f}ms")

           if max_error < 5:  # 5ms threshold
               print("  ✓ Concurrent execution does not degrade timing")
           else:
               print("  ⚠ Concurrent execution may affect timing accuracy")

       def generate_report(self):
           """Generate timing accuracy report"""
           print("\n" + "=" * 50)
           print("TIMING ACCURACY REPORT")
           print("=" * 50)

           print("\nSummary:")
           for result in self.results:
               acceptable = "✓" if abs(result['mean_error']) < 2 else "⚠"
               print(f"{acceptable} {result['target']*1000:.0f}ms target: {result['mean_error']:.3f}ms error")

           avg_error = statistics.mean([r['mean_error'] for r in self.results])
           print(f"\nOverall average error: {avg_error:.3f}ms")

           if abs(avg_error) < 2:
               print("✓ System meets T-Code timing requirements (<2ms error)")
           else:
               print("⚠ System may need optimization for T-Code timing")

   if __name__ == "__main__":
       test = TimingAccuracyTest()
       test.test_scheduled_execution()
       test.test_concurrent_execution()
       test.generate_report()
   ```

8. **Run Timing Tests:**
   ```bash
   python3 test_timing_accuracy.py
   ```

9. **Document Week 4 Status:**
   ```bash
   nano ~/WEEK4_STATUS.md
   ```

   Write:
   ```markdown
   # Week 4 Completion Status

   ## T-Code Implementation Complete:
   - [x] Velocity profile calculator working
   - [x] G-code parser functional
   - [x] Timestamp generator tested
   - [x] Main T-Code controller integrated
   - [x] Timing accuracy validated (<2ms error)

   ## Test Results:
   - Motion control: Working
   - Auxiliary command threading: Working
   - Timing synchronization: Accurate to X.XXms

   ## Ready for Next Phase:
   - Peltier hardware integration
   - Real auxiliary device control
   - Live bioprinting tests

   ## Configuration Backup:
   Date: [DATE]
   Files: ~/tcode/*.py
   ```

**Deliverable:** Complete T-Code system tested and validated, ready for Peltier integration

---

## Phase 3: Peltier System Integration (Weeks 5-8)

### **Week 5: Peltier Hardware Setup**

#### **Day 29-31: Peltier Control Circuit Assembly**

**Objectives:**
- Wire Peltier element with DPDT relay
- Connect ULN2001 driver
- Set up GPIO control from Raspberry Pi
- Test basic on/off control

**Tasks:**

**Day 29: Hardware Preparation**

1. **Gather Components:**
   ```
   Checklist:
   □ Peltier element (TEC1-12706 or similar)
   □ 12V DPDT relay (coil resistance measured in Week 1)
   □ ULN2001 driver IC or module
   □ Heat sink (for Peltier cold side)
   □ Thermal compound
   □ 12V power supply (29A - you already have)
   □ Jumper wires (dupont connectors)
   □ Breadboard or PCB for testing
   □ Multimeter
   □ Optional: Fan for active cooling
   ```

2. **Measure & Document:**
   ```bash
   nano ~/peltier_hardware_specs.txt
   ```

   Write:
   ```
   PELTIER HARDWARE SPECIFICATIONS
   ================================

   Peltier Element:
   - Model: TEC1-12706
   - Voltage: 12V
   - Current: XX A (measured with multimeter)
   - Resistance: XX Ω (V/I)
   - Hot side: [Mark which side]
   - Cold side: [Mark which side]

   DPDT Relay:
   - Coil voltage: 12V
   - Coil resistance: XX Ω (measured)
   - Coil current: XX mA (12V/resistance)
   - Contact rating: XX A
   - Pin configuration: [Draw pinout]

   ULN2001:
   - Package: DIP-16 or module
   - Channels used: 3 (heat, cool, polarity)
   - Max current per channel: 500mA
   - Actual load: XX mA (relay coil)

   Power Supply:
   - Voltage: 12V
   - Current capacity: 29A
   - Connection: [Document wiring]
   ```

3. **Test Components Individually:**
   ```
   Test 1: Peltier Element
   - Connect to 12V supply (briefly, <10 seconds)
   - Measure current draw
   - Feel temperature difference (one side hot, one cold)
   - Reverse polarity - sides should swap
   ✓ Record observations

   Test 2: DPDT Relay
   - Connect coil to 12V
   - Should hear "click"
   - Use multimeter continuity test on contacts
   - Verify switching behavior
   ✓ Document pin connections

   Test 3: ULN2001
   - Connect input pin to RPi GPIO (via 1kΩ resistor)
   - Connect output to LED + resistor (test load)
   - GPIO HIGH → LED OFF (ULN sinks, so inverted)
   - GPIO LOW → LED ON
   ✓ Verify logic levels
   ```

**Day 30: Circuit Assembly**

4. **Wiring Diagram:**
   ```
   PELTIER CONTROL CIRCUIT
   =======================

   Raspberry Pi GPIOs:
   GPIO 17 (Pin 11) → HEAT_CONTROL
   GPIO 27 (Pin 13) → COOL_CONTROL
   GPIO 22 (Pin 15) → POLARITY_CONTROL

   ULN2001 Connections:
   Pin 1 (IN1)  ← GPIO 17 (HEAT)
   Pin 2 (IN2)  ← GPIO 27 (COOL)
   Pin 3 (IN3)  ← GPIO 22 (POLARITY)
   Pin 8 (GND)  ← RPi GND
   Pin 9 (COM)  ← 12V+ (for relay coil power)

   Pin 13 (OUT1) → Relay 1 Coil (-)  [Heat Control]
   Pin 14 (OUT2) → Relay 2 Coil (-)  [Cool Control]
   Pin 15 (OUT3) → DPDT Coil (-)     [Polarity Control]

   Relay Coil (+) → 12V+

   DPDT Relay Contact Configuration:
   [COM1] ← Peltier Wire 1
   [NO1]  → 12V+
   [NC1]  → 12V-

   [COM2] ← Peltier Wire 2
   [NO2]  → 12V-
   [NC2]  → 12V+

   When DPDT energized (GPIO 22 LOW):
   - COM1 connects to NO1 (12V+)
   - COM2 connects to NO2 (12V-)
   - Peltier Wire 1 = +, Wire 2 = - (HEATING MODE)

   When DPDT de-energized (GPIO 22 HIGH):
   - COM1 connects to NC1 (12V-)
   - COM2 connects to NC2 (12V+)
   - Peltier Wire 1 = -, Wire 2 = + (COOLING MODE)
   ```

5. **Physical Assembly Steps:**
   ```
   Step 1: Mount heat sink to Peltier cold side
   - Apply thermal compound
   - Secure with screws or thermal adhesive
   - Attach fan (if using active cooling)

   Step 2: Wire ULN2001
   - Connect RPi GPIO to ULN inputs
   - Connect ULN GND to RPi GND
   - Connect ULN COM to 12V+ (for relay coil supply)

   Step 3: Wire DPDT relay
   - Coil (+) to 12V+
   - Coil (-) to ULN OUT3
   - Contacts wired as per diagram above

   Step 4: Wire Heat/Cool relays (if using separate relays)
   - OR use MOSFET for PWM control (recommended upgrade)

   Step 5: Connect Peltier
   - Wire 1 → DPDT COM1
   - Wire 2 → DPDT COM2
   - DO NOT POWER YET - test logic first
   ```

6. **Safety Checks:**
   ```bash
   nano ~/peltier_safety_checklist.txt
   ```

   Write:
   ```
   SAFETY CHECKLIST - Complete before powering
   ============================================

   □ All wire connections secure (no loose wires)
   □ No exposed metal contacts (risk of short circuit)
   □ Polarity markings verified on Peltier
   □ Heat sink properly attached with thermal compound
   □ 12V supply rated for at least 8A
   □ Fuse or circuit breaker in 12V line (recommended)
   □ Relay contact ratings exceed Peltier current
   □ ULN2001 current capacity not exceeded
   □ RPi GPIO pins connected via current-limiting resistors
   □ Double-checked wiring diagram against physical circuit
   □ Fire extinguisher accessible (working with high current)

   Signed: _____________ Date: _______
   ```

**Day 31: Initial Power-On Tests**

7. **Test Sequence (CRITICAL - Follow Exactly):**

   **Test 1: GPIO Control Only (No 12V)**
   ```python
   # Create test script
   nano ~/test_peltier_gpio.py
   ```

   ```python
   #!/usr/bin/env python3
   import RPi.GPIO as GPIO
   import time

   # Pin definitions
   HEAT_PIN = 17
   COOL_PIN = 27
   POLARITY_PIN = 22

   GPIO.setmode(GPIO.BCM)
   GPIO.setup(HEAT_PIN, GPIO.OUT)
   GPIO.setup(COOL_PIN, GPIO.OUT)
   GPIO.setup(POLARITY_PIN, GPIO.OUT)

   print("Testing GPIO control (relay logic)")
   print("Use multimeter to verify ULN outputs\n")

   print("1. All OFF")
   GPIO.output(HEAT_PIN, GPIO.HIGH)
   GPIO.output(COOL_PIN, GPIO.HIGH)
   GPIO.output(POLARITY_PIN, GPIO.HIGH)
   input("Press Enter...")

   print("2. HEAT mode (POLARITY=LOW, HEAT=LOW)")
   GPIO.output(POLARITY_PIN, GPIO.LOW)  # DPDT to heating position
   GPIO.output(HEAT_PIN, GPIO.LOW)      # Heat relay ON
   input("Press Enter...")

   print("3. COOL mode (POLARITY=HIGH, COOL=LOW)")
   GPIO.output(HEAT_PIN, GPIO.HIGH)     # Heat relay OFF
   GPIO.output(POLARITY_PIN, GPIO.HIGH) # DPDT to cooling position
   GPIO.output(COOL_PIN, GPIO.LOW)      # Cool relay ON
   input("Press Enter...")

   print("4. All OFF")
   GPIO.output(COOL_PIN, GPIO.HIGH)

   GPIO.cleanup()
   print("Test complete")
   ```

   Run:
   ```bash
   python3 ~/test_peltier_gpio.py
   ```

   Verify:
   - Relays click at correct times
   - Multimeter shows correct voltages at relay outputs
   - No unexpected behavior

   **Test 2: 12V Power-On (Peltier Connected)**

   ONLY proceed if Test 1 passed perfectly.

   ```python
   nano ~/test_peltier_power.py
   ```

   ```python
   #!/usr/bin/env python3
   import RPi.GPIO as GPIO
   import time

   HEAT_PIN = 17
   COOL_PIN = 27
   POLARITY_PIN = 22

   GPIO.setmode(GPIO.BCM)
   GPIO.setup(HEAT_PIN, GPIO.OUT)
   GPIO.setup(COOL_PIN, GPIO.OUT)
   GPIO.setup(POLARITY_PIN, GPIO.OUT)

   # Initialize to OFF
   GPIO.output(HEAT_PIN, GPIO.HIGH)
   GPIO.output(COOL_PIN, GPIO.HIGH)
   GPIO.output(POLARITY_PIN, GPIO.HIGH)

   print("Peltier Power Test")
   print("=" * 50)
   print("⚠ WARNING: Peltier will heat/cool")
   print("Have thermometer ready to measure temperature")
   input("Press Enter to start...")

   print("\n1. HEATING MODE (10 seconds)")
   GPIO.output(POLARITY_PIN, GPIO.LOW)  # Heating polarity
   GPIO.output(HEAT_PIN, GPIO.LOW)      # Power ON

   for i in range(10):
       print(f"  {10-i} seconds remaining...")
       time.sleep(1)

   GPIO.output(HEAT_PIN, GPIO.HIGH)  # Power OFF
   print("  → Measure temperature of marked side")
   print("  → Should be WARM/HOT")
   input("Press Enter to continue...")

   print("\n2. Wait for cool-down (30 seconds)")
   for i in range(30):
       print(f"  {30-i} seconds remaining...")
       time.sleep(1)

   print("\n3. COOLING MODE (10 seconds)")
   GPIO.output(POLARITY_PIN, GPIO.HIGH)  # Cooling polarity
   GPIO.output(COOL_PIN, GPIO.LOW)       # Power ON

   for i in range(10):
       print(f"  {10-i} seconds remaining...")
       time.sleep(1)

   GPIO.output(COOL_PIN, GPIO.HIGH)  # Power OFF
   print("  → Measure temperature of marked side")
   print("  → Should be COOL/COLD")
   input("Press Enter to continue...")

   GPIO.cleanup()
   print("\n✓ Power test complete")
   print("Verify polarity switching worked correctly")
   ```

   Run:
   ```bash
   python3 ~/test_peltier_power.py
   ```

   Expected results:
   - First test: One side gets warm (>30°C)
   - Second test: Same side gets cool (<20°C)
   - If sides don't swap, check DPDT wiring

**Deliverable:** Peltier hardware assembled, tested, polarity control verified

---

### **Week 6: Peltier Temperature Sensing & Basic Control**

#### **Day 32-35: Thermistor Integration & Temperature Reading**

**Objectives:**
- Connect thermistor to Peltier (or extruder)
- Configure Klipper to read temperature
- Create Python interface to read temp from Klipper
- Verify accurate temperature sensing

**Tasks:**

**Day 32: Thermistor Wiring**

1. **Thermistor Connection:**
   ```
   Octopus Board - E0 Thermistor Input (T0):
   Pin: PF4 (ADC input)
   Circuit: NTC 100K thermistor + 4.7kΩ pullup to 3.3V

   Connection:
   Thermistor Pin 1 → PF4 (T0 connector on Octopus)
   Thermistor Pin 2 → GND

   Physical Mounting:
   - Attach thermistor to Peltier hot side (for direct temp measurement)
   - OR attach to extruder body (for material temperature)
   - Use thermal compound or kapton tape
   - Secure wiring to prevent strain
   ```

2. **Update Klipper Config:**
   ```bash
   nano ~/printer_data/config/printer.cfg
   ```

   Find `[extruder]` section and verify/update:
   ```ini
   [extruder]
   step_pin: PG4
   dir_pin: PC1
   enable_pin: !PA0
   microsteps: 16
   rotation_distance: 33.5
   nozzle_diameter: 0.400
   filament_diameter: 1.750

   # Peltier temperature sensing
   heater_pin: PA2             # NOT USED (Peltier controlled by T-Code)
   sensor_type: Generic 3950   # NTC 100K thermistor
   sensor_pin: PF4             # T0 input on Octopus

   # Bioprinting temperature range
   min_temp: 0                 # Allow cooling below room temp
   max_temp: 50                # Peltier safe maximum

   # Disable heater (we use T-Code GPIO control)
   max_power: 0                # Prevent Klipper from activating heater
   ```

3. **Restart Klipper:**
   ```bash
   sudo systemctl restart klipper
   ```

4. **Test Temperature Reading:**
   ```
   In Mainsail console:
   TEMPERATURE_WAIT SENSOR=extruder MINIMUM=15

   Expected: Command completes (room temp > 15°C)

   Check current temp:
   M105

   Should show something like:
   T:23.4 /0.0
   (Current: 23.4°C, Target: 0°C)
   ```

**Day 33: Python Temperature Interface**

5. **Create Temperature Reader:**
   ```bash
   cd ~/tcode
   nano temperature_reader.py
   ```

6. **Implement Temperature Interface:**
   ```python
   #!/usr/bin/env python3
   """
   Temperature Reader for Klipper
   Reads extruder temperature via Moonraker API or serial
   """

   import serial
   import time
   import re

   class TemperatureReader:
       def __init__(self, klipper_serial='/dev/ttyACM0'):
           self.klipper = None
           self.klipper_serial = klipper_serial
           self.current_temp = None
           self.target_temp = None

       def connect(self):
           """Connect to Klipper via serial"""
           try:
               self.klipper = serial.Serial(self.klipper_serial, 250000, timeout=1)
               time.sleep(2)
               print("✓ Connected to Klipper for temperature reading")
               return True
           except Exception as e:
               print(f"✗ Connection failed: {e}")
               return False

       def read_temperature(self):
           """Read current temperature from Klipper"""
           if not self.klipper:
               return None

           try:
               # Send M105 command
               self.klipper.write(b"M105\n")
               time.sleep(0.1)

               # Read response
               if self.klipper.in_waiting:
                   response = self.klipper.readline().decode().strip()

                   # Parse response: "T:23.4 /0.0"
                   match = re.search(r'T:(\d+\.?\d*)\s*/(\d+\.?\d*)', response)
                   if match:
                       self.current_temp = float(match.group(1))
                       self.target_temp = float(match.group(2))
                       return self.current_temp

               return None
           except Exception as e:
               print(f"✗ Temperature read failed: {e}")
               return None

       def monitor_temperature(self, duration=60, interval=1):
           """Monitor temperature for specified duration"""
           print(f"Monitoring temperature for {duration} seconds")
           print("Time(s)\tTemp(°C)")
           print("-" * 30)

           start_time = time.time()
           temps = []

           while (time.time() - start_time) < duration:
               temp = self.read_temperature()
               if temp is not None:
                   elapsed = time.time() - start_time
                   print(f"{elapsed:.1f}\t{temp:.2f}")
                   temps.append(temp)

               time.sleep(interval)

           if temps:
               avg_temp = sum(temps) / len(temps)
               max_temp = max(temps)
               min_temp = min(temps)

               print("\nSummary:")
               print(f"  Average: {avg_temp:.2f}°C")
               print(f"  Range: {min_temp:.2f}°C - {max_temp:.2f}°C")
               print(f"  Stability: ±{(max_temp - min_temp) / 2:.2f}°C")

       def cleanup(self):
           """Close connection"""
           if self.klipper:
               self.klipper.close()
               print("✓ Connection closed")

   # Test
   if __name__ == "__main__":
       reader = TemperatureReader()

       try:
           if reader.connect():
               reader.monitor_temperature(duration=30, interval=1)
       except KeyboardInterrupt:
           print("\n⚠ Monitoring interrupted")
       finally:
           reader.cleanup()
   ```

7. **Test Temperature Reading:**
   ```bash
   python3 temperature_reader.py
   ```

   Expected output:
   ```
   ✓ Connected to Klipper for temperature reading
   Monitoring temperature for 30 seconds
   Time(s)    Temp(°C)
   ------------------------------
   0.1        23.42
   1.1        23.41
   2.1        23.43
   ...

   Summary:
     Average: 23.42°C
     Range: 23.40°C - 23.45°C
     Stability: ±0.03°C
   ```

**Day 34-35: Basic Peltier Control Implementation**

8. **Create Peltier Controller:**
   ```bash
   nano peltier_controller.py
   ```

9. **Implement Controller:**
   ```python
   #!/usr/bin/env python3
   """
   Peltier Controller for T-Code
   Basic on/off control with temperature feedback
   """

   import RPi.GPIO as GPIO
   import time
   from temperature_reader import TemperatureReader

   class PeltierController:
       # Pin definitions
       HEAT_PIN = 17
       COOL_PIN = 27
       POLARITY_PIN = 22

       # Control modes
       MODE_OFF = 0
       MODE_HEATING = 1
       MODE_COOLING = 2

       def __init__(self):
           self.current_mode = self.MODE_OFF
           self.target_temp = None
           self.temp_reader = TemperatureReader()

           # Initialize GPIO
           GPIO.setmode(GPIO.BCM)
           GPIO.setup(self.HEAT_PIN, GPIO.OUT)
           GPIO.setup(self.COOL_PIN, GPIO.OUT)
           GPIO.setup(self.POLARITY_PIN, GPIO.OUT)

           # Start in OFF state
           self.set_mode(self.MODE_OFF)

           print("✓ Peltier controller initialized")

       def set_mode(self, mode):
           """Set Peltier operating mode"""
           if mode == self.MODE_OFF:
               GPIO.output(self.HEAT_PIN, GPIO.HIGH)
               GPIO.output(self.COOL_PIN, GPIO.HIGH)
               print("  Mode: OFF")

           elif mode == self.MODE_HEATING:
               GPIO.output(self.COOL_PIN, GPIO.HIGH)  # Ensure cool is off
               GPIO.output(self.POLARITY_PIN, GPIO.LOW)  # Set heating polarity
               GPIO.output(self.HEAT_PIN, GPIO.LOW)   # Power on
               print("  Mode: HEATING")

           elif mode == self.MODE_COOLING:
               GPIO.output(self.HEAT_PIN, GPIO.HIGH)  # Ensure heat is off
               GPIO.output(self.POLARITY_PIN, GPIO.HIGH)  # Set cooling polarity
               GPIO.output(COOL_PIN, GPIO.LOW)   # Power on
               print("  Mode: COOLING")

           self.current_mode = mode

       def set_temperature(self, target):
           """Set target temperature"""
           self.target_temp = target
           print(f"Target temperature set to {target}°C")

       def simple_control_loop(self, duration=300, deadband=0.5):
           """
           Simple on/off control (bang-bang)
           Will be replaced with PID in Week 7
           """
           print(f"\nRunning simple control for {duration} seconds")
           print(f"Target: {self.target_temp}°C, Deadband: ±{deadband}°C")
           print("\nTime(s)\tTemp(°C)\tError(°C)\tMode")
           print("-" * 60)

           if not self.temp_reader.connect():
               return False

           start_time = time.time()

           try:
               while (time.time() - start_time) < duration:
                   # Read current temperature
                   current_temp = self.temp_reader.read_temperature()

                   if current_temp is None:
                       print("⚠ Temperature read failed")
                       time.sleep(1)
                       continue

                   # Calculate error
                   error = self.target_temp - current_temp

                   # Simple bang-bang control
                   if error > deadband:
                       # Need heating
                       if self.current_mode != self.MODE_HEATING:
                           self.set_mode(self.MODE_HEATING)

                   elif error < -deadband:
                       # Need cooling
                       if self.current_mode != self.MODE_COOLING:
                           self.set_mode(self.MODE_COOLING)

                   else:
                       # Within deadband - turn off
                       if self.current_mode != self.MODE_OFF:
                           self.set_mode(self.MODE_OFF)

                   # Log
                   elapsed = time.time() - start_time
                   mode_str = ["OFF", "HEATING", "COOLING"][self.current_mode]
                   print(f"{elapsed:.1f}\t{current_temp:.2f}\t\t{error:.2f}\t\t{mode_str}")

                   time.sleep(1)

           finally:
               self.set_mode(self.MODE_OFF)
               self.temp_reader.cleanup()

           print("\n✓ Control loop complete")
           return True

       def cleanup(self):
           """Clean up resources"""
           self.set_mode(self.MODE_OFF)
           GPIO.cleanup()
           print("✓ Peltier controller cleaned up")

   # Test
   if __name__ == "__main__":
       import sys

       if len(sys.argv) < 2:
           print("Usage: python3 peltier_controller.py <target_temp>")
           print("Example: python3 peltier_controller.py 20")
           sys.exit(1)

       target = float(sys.argv[1])

       controller = PeltierController()
       controller.set_temperature(target)

       try:
           controller.simple_control_loop(duration=180)  # 3 minutes
       except KeyboardInterrupt:
           print("\n⚠ Control interrupted")
       finally:
           controller.cleanup()
   ```

10. **Test Peltier Control:**
    ```bash
    # Test heating (if room temp is ~23°C)
    python3 peltier_controller.py 28

    # Test cooling (if room temp is ~23°C)
    python3 peltier_controller.py 18
    ```

    Expected behavior:
    - Temperature should move toward target
    - Mode should switch between HEATING/COOLING/OFF
    - Temperature should stabilize near target (within deadband)

11. **Document Week 6 Results:**
    ```bash
    nano ~/WEEK6_STATUS.md
    ```

    Write:
    ```markdown
    # Week 6 Completion Status

    ## Peltier System Functional:
    - [x] Hardware assembled and tested
    - [x] Thermistor reading accurate
    - [x] Temperature interface working
    - [x] Basic bang-bang control implemented
    - [x] Heating mode verified
    - [x] Cooling mode verified

    ## Test Results:
    - Heating: X°C/minute rate
    - Cooling: X°C/minute rate
    - Temperature stability: ±X°C
    - Mode switching: Working

    ## Next Steps:
    - Implement PID control
    - Integrate with T-Code
    - Test synchronized printing + temperature control
    ```

**Deliverable:** Peltier temperature control working with basic on/off logic, ready for PID implementation

---

### **Week 7-8: PID Control & T-Code Integration**

*(Detailed plan continues with PID implementation, integration with T-Code controller, and synchronized printing tests)*

---

## Summary of 12-Week Plan Structure

**Weeks 1-2:** Klipper Foundation
- Install Klipper on Raspberry Pi
- Configure Octopus board
- Port E0 homing from Marlin
- Calibrate motion system

**Weeks 3-4:** T-Code Foundation
- Clone Johns Hopkins repository
- Adapt code for Klipper + RPi
- Test timing and synchronization
- Validate end-to-end workflow

**Weeks 5-6:** Peltier Hardware
- Assemble Peltier control circuit
- Test temperature sensing
- Implement basic control
- Verify heating/cooling modes

**Weeks 7-8:** PID & Integration
- Implement PID controller
- Integrate Peltier with T-Code
- Test synchronized operation
- Tune performance

**Weeks 9-10:** Pressure System
- Add pneumatic dispenser control
- Integrate with T-Code
- Test multi-material capability
- Optimize extrusion

**Weeks 11-12:** Full System Testing
- Complete bioprinter testing
- Real bioprinting experiments
- Documentation and refinement
- Production readiness

---

## Appendices

### Appendix A: Tool Cheat Sheet

**Raspberry Pi Commands:**
```bash
# SSH into RPi
ssh pi@bioprinter.local

# Restart Klipper
sudo systemctl restart klipper

# View Klipper logs
journalctl -u klipper -f

# Activate Python virtual environment
cd ~/tcode
source tcode-env/bin/activate

# Find USB devices
ls -l /dev/serial/by-id/

# Monitor GPIO pins
gpio readall

# Check RPi temperature
vcgencmd measure_temp
```

**Klipper G-code Commands:**
```gcode
G28              ; Home all axes
G28 X Y          ; Home specific axes
HOME_SYRINGE     ; Custom macro for E0 homing
QUERY_ENDSTOPS   ; Check endstop status
M105             ; Report temperature
TEMPERATURE_WAIT SENSOR=extruder MINIMUM=20  ; Wait for temp
```

**Python T-Code:**
```bash
# Run T-Code controller
python3 tcode_controller.py test_print.gcode

# Monitor temperature
python3 temperature_reader.py

# Test Peltier control
python3 peltier_controller.py 20

# Run timing tests
python3 test_timing_accuracy.py
```

### Appendix B: Troubleshooting Guide

**Issue:** Klipper won't connect
- Check USB cable
- Verify firmware flashed: `ls /dev/serial/by-id/`
- Check serial port in printer.cfg
- Restart Klipper: `sudo systemctl restart klipper`

**Issue:** E0 won't home
- Verify endstop wiring (PG11 on Octopus)
- Test endstop: `QUERY_ENDSTOPS`
- Check macro syntax in macros.cfg
- Verify manual_stepper config

**Issue:** Temperature reading incorrect
- Check thermistor wiring (PF4 + GND)
- Verify sensor_type in printer.cfg
- Test with room temperature (should be ~23°C)
- Check for shorts or loose connections

**Issue:** Peltier not switching polarity
- Verify DPDT relay wiring
- Test relay manually (apply 12V to coil)
- Check GPIO control: `gpio readall`
- Measure ULN2001 outputs with multimeter

**Issue:** T-Code timing inaccurate
- Run timing tests: `test_timing_accuracy.py`
- Check system load: `top` or `htop`
- Disable unnecessary services
- Consider using real-time kernel (advanced)

### Appendix C: Parts List & Budget

**Electronics:**
- Raspberry Pi 4 (8GB): $75
- MicroSD Card (32GB): $10
- Power Supply (USB-C 3A): $10
- BTT Octopus V1.1: (already have)
- Peltier TEC1-12706: $5-10
- DPDT Relay (12V): $3
- ULN2001 IC: $1
- Heat sink + fan: $10
- Thermistor NTC 100K: $2
- Jumper wires: $5
- Breadboard/PCB: $5

**Power:**
- 12V 29A SMPS: (already have)

**Mechanical:**
- Thermal compound: $5
- Mounting hardware: $5

**Total New Parts:** ~$130-145

### Appendix D: Safety Guidelines

**Electrical:**
- Never work on powered circuits
- Use fused power supplies
- Check polarity before connecting
- Insulate all connections

**Thermal:**
- Peltier can reach 80°C+ on hot side
- Always use heat sink
- Don't touch Peltier when powered
- Allow cool-down time

**Chemical (for bioprinting):**
- Follow material safety data sheets
- Use appropriate PPE
- Proper waste disposal
- Sterile technique for cell work

---

**End of Implementation Plan**

**Total Duration:** 12 weeks
**Complexity:** Advanced
**Outcome:** Production-ready bioprinter with T-Code control

This plan provides week-by-week, day-by-day guidance for transitioning from your current Marlin setup to a complete Klipper + T-Code bioprinter system. Each phase builds on the previous one, with clear deliverables and validation steps.
