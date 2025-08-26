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
//  FlagManager Example: 04_ISR_Communication
//  Demonstrates the correct and safe way to modify flags
//  from within an Interrupt Service Routine (ISR) and read
//  them in the main loop.
//===========================================================
#include "flagManager.h"
#include <cstdint>

// --- Hardware Definitions ---
// Note: These pins are chosen for boards with many interrupt-
// capable pins, like an Arduino Mega or Teensy. An Arduino Uno
// only has two external interrupt pins (2 and 3).
const int inputPins[] = {34, 35, 36, 37, 38, 39, 40, 41};
const int ledPins[] = {2, 3, 4, 5, 6, 7, 8, 9};
const int numPins = 8;

// --- Global Volatile Flag Manager ---
// The 'volatile' keyword is CRITICAL. It tells the compiler that this
// variable can be changed at any time by an external event (like an
// ISR), preventing the compiler from making optimizations that could
// cause the program to miss updates to the variable.
volatile FlagManager<uint8_t> inputFlags;

//===========================================================
//  INTERRUPT SERVICE ROUTINES (ISRs)
//  These functions are called by the hardware when a pin's
//  state changes. They must be as fast as possible. Using
//  toggleFlag() is an extremely fast, single-cycle operation.
//===========================================================
void isrForPin34() { inputFlags.toggleFlag(0); }
void isrForPin35() { inputFlags.toggleFlag(1); }
void isrForPin36() { inputFlags.toggleFlag(2); }
void isrForPin37() { inputFlags.toggleFlag(3); }
void isrForPin38() { inputFlags.toggleFlag(4); }
void isrForPin39() { inputFlags.toggleFlag(5); }
void isrForPin40() { inputFlags.toggleFlag(6); }
void isrForPin41() { inputFlags.toggleFlag(7); }

void setup() {
  /**
   * @brief Configures pins and attaches the ISRs to hardware interrupts.
   */
  Serial.begin(115200);
  while (!Serial);
  Serial.println("--- FlagManager ISR Communication Demo ---");
  Serial.println("Toggle inputs to change corresponding LEDs.");

  // Configure pins and attach interrupts.
  for (int i = 0; i < numPins; i++) {
    // Set input pins with an internal pull-up resistor (for buttons).
    pinMode(inputPins[i], INPUT_PULLUP);
    // Set LED pins to output.
    pinMode(ledPins[i], OUTPUT);
  }

  // Attach the ISR functions to the corresponding hardware interrupts.
  // The 'CHANGE' mode triggers the ISR on both rising and falling edges.
  attachInterrupt(digitalPinToInterrupt(inputPins[0]), isrForPin34, CHANGE);
  attachInterrupt(digitalPinToInterrupt(inputPins[1]), isrForPin35, CHANGE);
  attachInterrupt(digitalPinToInterrupt(inputPins[2]), isrForPin36, CHANGE);
  attachInterrupt(digitalPinToInterrupt(inputPins[3]), isrForPin37, CHANGE);
  attachInterrupt(digitalPinToInterrupt(inputPins[4]), isrForPin38, CHANGE);
  attachInterrupt(digitalPinToInterrupt(inputPins[5]), isrForPin39, CHANGE);
  attachInterrupt(digitalPinToInterrupt(inputPins[6]), isrForPin40, CHANGE);
  attachInterrupt(digitalPinToInterrupt(inputPins[7]), isrForPin41, CHANGE);
}

void loop() {
  /**
   * @brief Safely reads the volatile flag data and updates LEDs.
   */
  uint8_t flagsCopy;
  static uint8_t previousFlags = 0;

  // --- Create a "Critical Section" to Safely Read Volatile Data ---
  // To prevent a race condition where an ISR might change `inputFlags`
  // while we are reading it, we temporarily disable interrupts,
  // make a quick copy, and then immediately re-enable them.
  noInterrupts();
  flagsCopy = inputFlags.getRawFlags(); // Assumes getRawFlags() returns the underlying uint8_t
  interrupts();

  // Only update LEDs and print to Serial if the flags have actually changed.
  if (flagsCopy != previousFlags) {
    Serial.print("Flags changed: ");
    // We can now work with our local, non-volatile copy without worrying
    // about it being changed unexpectedly.
    for (int i = 0; i < numPins; i++) {
      bool ledState = bitRead(flagsCopy, i);
      digitalWrite(ledPins[i], ledState);
      Serial.print(ledState); // Print the new state
    }
    Serial.println();
    previousFlags = flagsCopy; // Update the previous state
  }

  // The main loop is now free to perform other non-blocking tasks.
}
