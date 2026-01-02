/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2024 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Pneumatic Extruder Control for E1 - Implementation
 */

#include "../inc/MarlinConfig.h"

#ifdef PNEUMATIC_EXTRUDER_E1

#include "pneumatic_extruder.h"
#include "../module/stepper.h"
#include "../module/planner.h"
#include "../HAL/shared/Delay.h"

PneumaticExtruder pneumatic_e1;

bool PneumaticExtruder::is_active = false;
bool PneumaticExtruder::is_extruding = false;
uint32_t PneumaticExtruder::extrusion_start_ms = 0;

/**
 * Initialize pneumatic extruder control
 * Sets PC3 (E1_ENABLE_PIN) as output and ensures it starts LOW (valve closed)
 */
void PneumaticExtruder::init() {
  #if PIN_EXISTS(E1_ENABLE)
    OUT_WRITE(E1_ENABLE_PIN, LOW);  // PC3 = LOW (valve closed, no dispensing)
  #endif

  is_active = false;
  is_extruding = false;

  SERIAL_ECHOLNPGM("Pneumatic Extruder E1: Initialized on pin PC3");
}

/**
 * Handle tool change events
 * Called when switching between T0 (stepper) and T1 (pneumatic)
 */
void PneumaticExtruder::on_tool_change(const uint8_t new_extruder) {
  is_active = (new_extruder == 1);  // E1 is extruder index 1

  // Ensure valve is closed when switching tools
  if (!is_active && is_extruding) {
    stop_extrusion();
  }

  #if ENABLED(DEBUG_PNEUMATIC_EXTRUDER)
    SERIAL_ECHOPGM("Pneumatic E1: Tool change to T");
    SERIAL_ECHO(new_extruder);
    SERIAL_ECHOLNPGM(is_active ? " (PNEUMATIC ACTIVE)" : " (PNEUMATIC INACTIVE)");
  #endif
}

/**
 * Start extrusion - Opens pneumatic valve
 * Called when beginning an extrusion move with E1 active
 */
void PneumaticExtruder::start_extrusion() {
  if (!is_active) return;  // Only control valve if E1 is selected

  #if PIN_EXISTS(E1_ENABLE)
    WRITE(E1_ENABLE_PIN, HIGH);  // PC3 = HIGH (3.3V, valve open, dispense)
  #endif

  is_extruding = true;
  extrusion_start_ms = millis();

  #if ENABLED(DEBUG_PNEUMATIC_EXTRUDER)
    SERIAL_ECHOLNPGM("Pneumatic E1: Valve OPEN (extruding)");
  #endif
}

/**
 * Stop extrusion - Closes pneumatic valve
 * Called when extrusion move completes
 */
void PneumaticExtruder::stop_extrusion() {
  if (!is_active && !is_extruding) return;

  #if PIN_EXISTS(E1_ENABLE)
    WRITE(E1_ENABLE_PIN, LOW);   // PC3 = LOW (0V, valve closed)
  #endif

  is_extruding = false;

  #if ENABLED(DEBUG_PNEUMATIC_EXTRUDER)
    const uint32_t duration = millis() - extrusion_start_ms;
    SERIAL_ECHOPGM("Pneumatic E1: Valve CLOSED (duration: ");
    SERIAL_ECHO(duration);
    SERIAL_ECHOLNPGM("ms)");
  #endif
}

/**
 * Update pneumatic valve state
 * This is called periodically to manage valve timing
 */
void PneumaticExtruder::update() {
  // Currently valve state is managed by start/stop functions
  // This function is available for future timed pressure control
}

/**
 * Manual valve control (for testing via G-code M42 or custom M-code)
 */
void PneumaticExtruder::set_valve(const bool state) {
  #if PIN_EXISTS(E1_ENABLE)
    WRITE(E1_ENABLE_PIN, state);
  #endif

  is_extruding = state;

  if (state) {
    extrusion_start_ms = millis();
  }

  SERIAL_ECHOPGM("Pneumatic E1: Valve ");
  SERIAL_ECHOLNPGM(state ? "OPEN (manual)" : "CLOSED (manual)");
}

#endif // PNEUMATIC_EXTRUDER_E1
