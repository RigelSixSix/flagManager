//===========================================================
//  FlagManager
//  Provides a fast and efficient means of setting and
//  interacting with up to 32 flags (bits) per instance. This templated
//  version works with uint8_t, uint16_t, and uint32_t.
//  The MCU should be connected to 16 LEDs in 2 banks of 8 to
//  visualise the effect
//
//  Don Gibson
//  Greybeard Precision
//  Vancouver, Canada
//  V2.10 August 2025
//
//  FlagManager Example: 05_System_State_Simulator
//  Demonstrates using multiple FlagManager instances to model
//  and simulate the state of a complex control system with
//  interdependent subsystems.
//===========================================================
#include "flagManager.h"
#include <cstdint>

//===========================================================
//  SYSTEM STATE DEFINITIONS
//===========================================================

// --- Using namespaces to scope enums and prevent name conflicts ---

// -- Power Subsystem --
namespace PowerFlags {
  enum {
    mainPower,      // Is the main system power on?
    backupPower,    // Is the system on battery backup?
    solarCharging,  // Is the solar panel charging the battery?
    gridFault,      // Has a fault been detected on the main power grid?
    stateOK = 7     // Is this subsystem in a safe state?
  };
}
FlagManager<uint8_t> powerState;

// -- Cooling Subsystem --
namespace CoolingFlags {
  enum {
    coolingFan1,    // Fan 1 active
    coolingFan2,    // Fan 2 active
    line1OverTemp,  // Sensor: Line 1 is too hot
    line1UnderTemp, // Sensor: Line 1 is too cold
    stateOK = 7     // Is this subsystem in a safe state?
  };
}
FlagManager<uint8_t> coolingState;

// -- Pump Subsystem --
namespace PumpFlags {
  enum {
    pump1Active,    // Pump 1 is running
    pump2Active,    // Pump 2 is running
    tank1Full,      // Sensor: Tank 1 is full
    tank2Empty,     // Sensor: Tank 2 is empty
    stateOK = 7     // Is this subsystem in a safe state?
  };
}
FlagManager<uint8_t> pumpState;

//===========================================================
//  HARDWARE & SIMULATION DEFINITIONS
//===========================================================

const int ledPins[] = {2, 3, 4, 5, 6, 7, 8, 9};
enum LedMap {
  LED_SYSTEM_OK,      // LED 0: (GREEN) Lit if all systems are OK
  LED_MAIN_POWER,     // LED 1: System has main power
  LED_PUMP_1,         // LED 2: Pump 1 is active
  LED_FAN_1,          // LED 3: Fan 1 is active
  LED_UNUSED_1,       // LED 4
  LED_UNUSED_2,       // LED 5
  LED_TEMP_FAULT,     // LED 6: (Blinking) Over/Under temperature fault
  LED_GENERAL_FAULT   // LED 7: (Solid) Any other fault (grid, tank empty)
};

const unsigned long CYCLE_DURATION = 20000; // 20 seconds for a full cycle
unsigned long cycleStartTime = 0;

/**
 * @brief Resets all system flags to a known, healthy starting state.
 */
void resetSimulation() {
  Serial.println("\n--- SIMULATION CYCLE RESET ---");
  // Clear all flags in every subsystem
  powerState.clearAllFlags();
  coolingState.clearAllFlags();
  pumpState.clearAllFlags();

  // Set initial 'OK' states
  powerState.setFlag(PowerFlags::mainPower);
  powerState.setFlag(PowerFlags::stateOK);
  coolingState.setFlag(CoolingFlags::stateOK);
  pumpState.setFlag(PumpFlags::stateOK);
  Serial.println("[0ms] All systems OK. Main power enabled.");
}

/**
 * @brief Updates the physical LEDs on the control panel based on current flags.
 */
void updateControlPanel() {
  // Check the overall system health by checking the 'stateOK' flag of each subsystem.
  bool systemOk = powerState.checkFlag(PowerFlags::stateOK) &&
                  coolingState.checkFlag(CoolingFlags::stateOK) &&
                  pumpState.checkFlag(PumpFlags::stateOK);

  digitalWrite(ledPins[LED_SYSTEM_OK], systemOk);

  // Update indicators for specific components.
  digitalWrite(ledPins[LED_MAIN_POWER], powerState.checkFlag(PowerFlags::mainPower));
  digitalWrite(ledPins[LED_PUMP_1],     pumpState.checkFlag(PumpFlags::pump1Active));
  digitalWrite(ledPins[LED_FAN_1],      coolingState.checkFlag(CoolingFlags::coolingFan1));

  // Handle fault indicators.
  bool tempFault = coolingState.checkFlag(CoolingFlags::line1OverTemp) || coolingState.checkFlag(CoolingFlags::line1UnderTemp);
  bool generalFault = powerState.checkFlag(PowerFlags::gridFault) || pumpState.checkFlag(PumpFlags::tank2Empty);

  // Make the temp fault LED blink.
  if (tempFault) {
    digitalWrite(ledPins[LED_TEMP_FAULT], (millis() / 200) % 2);
  } else {
    digitalWrite(ledPins[LED_TEMP_FAULT], LOW);
  }
  digitalWrite(ledPins[LED_GENERAL_FAULT], generalFault);
}

void setup() {
  /**
   * @brief Initializes serial port, configures LED pins, and starts the simulation.
   */
  Serial.begin(115200);
  while (!Serial);
  for (int pin : ledPins) {
    pinMode(pin, OUTPUT);
  }
  resetSimulation();
  cycleStartTime = millis();
}

void loop() {
  /**
   * @brief Runs the simulation by checking for timed events and updating the panel.
   */
  unsigned long timeInCycle = millis() - cycleStartTime;

  // --- Event Timeline ---
  // A series of non-blocking checks simulate events occurring over time.

  // Event 1: (At 2s) Start Pump 1
  if (timeInCycle > 2000 && !pumpState.checkFlag(PumpFlags::pump1Active)) {
    pumpState.setFlag(PumpFlags::pump1Active);
    Serial.println("[2000ms] EVENT: Starting Pump 1.");
  }

  // Event 2: (At 5s) Temperature rises, turn on Fan 1
  if (timeInCycle > 5000 && !coolingState.checkFlag(CoolingFlags::coolingFan1)) {
    coolingState.setFlag(CoolingFlags::coolingFan1);
    Serial.println("[5000ms] INFO: Temperature rising. Activating Fan 1.");
  }

  // Event 3: (At 8s) Critical temperature fault!
  if (timeInCycle > 8000 && coolingState.checkFlag(CoolingFlags::stateOK)) {
    coolingState.setFlag(CoolingFlags::line1OverTemp);
    coolingState.clearFlag(CoolingFlags::stateOK); // <-- State is no longer OK
    Serial.println("[8000ms] !! FAULT: Line 1 Over Temperature! Cooling system state: NOT OK.");
  }

  // Event 4: (At 12s) Grid power fails, switch to backup
  if (timeInCycle > 12000 && powerState.checkFlag(PowerFlags::stateOK)) {
    powerState.setFlag(PowerFlags::gridFault);
    powerState.clearFlag(PowerFlags::mainPower);
    powerState.setFlag(PowerFlags::backupPower);
    powerState.clearFlag(PowerFlags::stateOK); // <-- State is no longer OK
    Serial.println("[12000ms] !! FAULT: Grid power failure! Power system state: NOT OK.");
  }

  // Event 5: (At 16s) System resolves over-temp, but pump runs tank dry
  if (timeInCycle > 16000 && !coolingState.checkFlag(CoolingFlags::stateOK)) {
    coolingState.clearFlag(CoolingFlags::line1OverTemp);
    coolingState.setFlag(CoolingFlags::stateOK); // <-- Cooling system is OK again
    pumpState.setFlag(PumpFlags::tank2Empty);
    pumpState.clearFlag(PumpFlags::stateOK);     // <-- Pump system is now NOT OK
    pumpState.clearFlag(PumpFlags::pump1Active);
    Serial.println("[16000ms] INFO: Over-temperature resolved. Cooling system: OK.");
    Serial.println("[16000ms] !! FAULT: Tank 2 is empty! Pump system state: NOT OK.");
  }

  // Continuously update the control panel LEDs.
  updateControlPanel();

  // Check if it's time to reset the whole cycle.
  if (timeInCycle >= CYCLE_DURATION) {
    cycleStartTime = millis();
    resetSimulation();
  }
}
