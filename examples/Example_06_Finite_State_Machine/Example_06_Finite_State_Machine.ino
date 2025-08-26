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
//  FlagManager Example: 06_Finite_State_Machine
//  Demonstrates a sophisticated control system using a single
//  32-bit FlagManager to hold the entire system's state,
//  managed by a Finite State Machine (FSM).
//===========================================================
#include "flagManager.h"
#include <cstdint>

//===========================================================
//  CONTROL SYSTEM DEFINITIONS
//===========================================================

namespace ControlSystem {
  // A single 32-bit manager holds the entire system state.
  FlagManager<uint32_t> systemFlags;

  // --- A single, consolidated set of flags for the entire system ---
  namespace Flags {
    enum {
      // Power Subsystem Flags (Bits 0-7)
      FLAG_MAIN_POWER_OK,
      FLAG_BACKUP_POWER_ACTIVE,
      FLAG_GRID_FAULT,

      // Cooling Subsystem Flags (Bits 8-15)
      FLAG_COOLING_FAN1_ACTIVE = 8,
      FLAG_COOLING_FAN2_ACTIVE,
      FLAG_OVERTEMP_FAULT,

      // Pump Subsystem Flags (Bits 16-23)
      FLAG_PUMP1_ACTIVE = 16,
      FLAG_TANK2_EMPTY_FAULT
    };
  }

  // --- Defines the states for our Finite State Machine (FSM) ---
  enum class State {
    IDLE,
    NORMAL_OPERATION,
    HIGH_TEMP_OPERATION,
    OVERTEMP_FAULT,
    GRID_FAILURE
  };

  // --- Hardware pin definitions and mapping ---
  namespace Hardware {
    const int ledPins[] = {2, 3, 4, 5, 6, 7, 8, 9};
    enum LedMap {
      LED_MAIN_POWER,
      LED_BACKUP_POWER,
      LED_PUMP_1,
      LED_FAN_1,
      LED_FAN_2,
      LED_UNUSED,
      LED_OVERTEMP,
      LED_GRID_FAULT
    };
  }
} // end namespace ControlSystem

// "Import" the ControlSystem namespace for cleaner code in the sketch.
using namespace ControlSystem;

/**
 * @brief Updates the physical LEDs on the control panel based on current flags.
 */
void updateControlPanel() {
  digitalWrite(Hardware::ledPins[Hardware::LED_MAIN_POWER],   systemFlags.checkFlag(Flags::FLAG_MAIN_POWER_OK));
  digitalWrite(Hardware::ledPins[Hardware::LED_BACKUP_POWER], systemFlags.checkFlag(Flags::FLAG_BACKUP_POWER_ACTIVE));
  digitalWrite(Hardware::ledPins[Hardware::LED_PUMP_1],       systemFlags.checkFlag(Flags::FLAG_PUMP1_ACTIVE));
  digitalWrite(Hardware::ledPins[Hardware::LED_FAN_1],        systemFlags.checkFlag(Flags::FLAG_COOLING_FAN1_ACTIVE));
  digitalWrite(Hardware::ledPins[Hardware::LED_FAN_2],        systemFlags.checkFlag(Flags::FLAG_COOLING_FAN2_ACTIVE));

  // Blink the LED if an overtemp fault is active.
  bool overtemp = systemFlags.checkFlag(Flags::FLAG_OVERTEMP_FAULT);
  digitalWrite(Hardware::ledPins[Hardware::LED_OVERTEMP], overtemp && (millis() / 200) % 2);

  // Blink the LED if a grid fault is active.
  bool gridFault = systemFlags.checkFlag(Flags::FLAG_GRID_FAULT);
  digitalWrite(Hardware::ledPins[Hardware::LED_GRID_FAULT], gridFault && (millis() / 200) % 2);
}

void setup() {
  /**
   * @brief Initializes serial port and configures LED pins.
   */
  Serial.begin(115200);
  while (!Serial);
  for (int pin : Hardware::ledPins) {
    pinMode(pin, OUTPUT);
  }
}

//===========================================================
//  LOOP - FINITE STATE MACHINE
//===========================================================
void loop() {
  /**
   * @brief Runs the control system using a Finite State Machine (FSM).
   * The FSM transitions between states based on timed events, and each
   * state is responsible for setting the appropriate flags.
   */
  const unsigned long CYCLE_DURATION = 20000;
  unsigned long timeInCycle = millis() % CYCLE_DURATION;
  static State currentState = State::IDLE;
  static unsigned long previousTimeInCycle = 0;

  // Reset the state machine when the simulation cycle restarts.
  if (timeInCycle < previousTimeInCycle) {
    currentState = State::IDLE;
  }
  previousTimeInCycle = timeInCycle;

  // The core of the FSM is this switch statement.
  switch (currentState) {

    case State::IDLE:
      Serial.println("\n--- CYCLE START (IDLE) ---");
      systemFlags.clearAllFlags();
      systemFlags.setFlag(Flags::FLAG_MAIN_POWER_OK);
      Serial.println("System Online. Main power enabled.");
      currentState = State::NORMAL_OPERATION; // Transition to the next state
      break;

    case State::NORMAL_OPERATION:
      if (!systemFlags.checkFlag(Flags::FLAG_PUMP1_ACTIVE)) {
        systemFlags.setFlag(Flags::FLAG_PUMP1_ACTIVE);
        Serial.println("STATE: Normal Operation. Pump 1 activated.");
      }
      // Transition condition:
      if (timeInCycle > 5000) {
        currentState = State::HIGH_TEMP_OPERATION;
      }
      break;

    case State::HIGH_TEMP_OPERATION:
      if (!systemFlags.checkFlag(Flags::FLAG_COOLING_FAN1_ACTIVE)) {
        systemFlags.setFlag(Flags::FLAG_COOLING_FAN1_ACTIVE);
        systemFlags.setFlag(Flags::FLAG_COOLING_FAN2_ACTIVE);
        Serial.println("STATE: High Temp Operation. Dual Fans activated.");
      }
      // Transition condition:
      if (timeInCycle > 8000) {
        currentState = State::OVERTEMP_FAULT;
      }
      break;

    case State::OVERTEMP_FAULT:
      if (!systemFlags.checkFlag(Flags::FLAG_OVERTEMP_FAULT)) {
        systemFlags.setFlag(Flags::FLAG_OVERTEMP_FAULT);
        Serial.println("!! STATE: Overtemp FAULT !! Fans at max, monitoring.");
      }
      // Transition condition:
      if (timeInCycle > 12000) {
        currentState = State::GRID_FAILURE;
      }
      break;

    case State::GRID_FAILURE:
      if (!systemFlags.checkFlag(Flags::FLAG_GRID_FAULT)) {
        systemFlags.setFlag(Flags::FLAG_GRID_FAULT);
        systemFlags.clearFlag(Flags::FLAG_MAIN_POWER_OK);
        systemFlags.setFlag(Flags::FLAG_BACKUP_POWER_ACTIVE);
        Serial.println("!! STATE: Grid Failure FAULT !! Switching to battery backup.");
        // This is the final state for this demonstration cycle.
      }
      break;
  }

  updateControlPanel();
  delay(10);
}
