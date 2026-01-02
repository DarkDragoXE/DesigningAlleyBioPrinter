/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2024 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Pneumatic Extruder Control for E1
 *
 * This feature converts E1 from a stepper-based extruder to a pneumatic dispenser.
 * The E1_ENABLE_PIN (PC3) acts as a solenoid/valve control that goes HIGH during
 * extrusion commands.
 *
 * Hardware Connection:
 * - PC3 (E1_ENABLE_PIN) → Pneumatic control board signal input
 * - When extruding: PC3 = HIGH (3.3V, pneumatic valve OPEN, dispenses material)
 * - When not extruding: PC3 = LOW (0V, pneumatic valve CLOSED)
 *
 * Usage:
 * - T0: Selects E0 (stepper motor for syringe-based bioink)
 * - T1: Selects E1 (pneumatic dispenser)
 * - G1 E10 F300: Extrudes 10mm at 300mm/min
 *   - If T0 active: E0 motor rotates
 *   - If T1 active: PC3 goes HIGH (3.3V) for duration of move
 */

#pragma once

#include "../inc/MarlinConfig.h"

// Enable pneumatic control for E1
#define PNEUMATIC_EXTRUDER_E1

#ifdef PNEUMATIC_EXTRUDER_E1

class PneumaticExtruder {
public:
  static bool is_active;              // Is E1 currently selected as active extruder?
  static bool is_extruding;           // Is E1 currently extruding?
  static uint32_t extrusion_start_ms; // When did extrusion start (for timing control)

  // Initialize pneumatic control
  static void init();

  // Called when extruder changes (T0/T1 commands)
  static void on_tool_change(const uint8_t new_extruder);

  // Called at start of extrusion move for E1
  static void start_extrusion();

  // Called at end of extrusion move for E1
  static void stop_extrusion();

  // Update pneumatic valve state (call from stepper ISR or main loop)
  static void update();

  // Manual control (for testing)
  static void set_valve(const bool state);
};

extern PneumaticExtruder pneumatic_e1;

#endif // PNEUMATIC_EXTRUDER_E1
