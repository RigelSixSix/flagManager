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
//  FlagManager Example: 02_Cylon_Scanner
//  Demonstrates using 8-bit FlagManager instances to control
//  two banks of LEDs, creating a "Cylon scanner" visual
//  effect.
//===========================================================
#include "flagManager.h"
#include <cstdint>

// Create an 8-bit flag manager for each bank of 8 LEDs.
// Each flag (0-7) will correspond to an individual LED.
FlagManager<uint8_t> ledBank1Flags;
FlagManager<uint8_t> ledBank2Flags;


// Define the LED pins for each bank
const int ledPinsBank1[] = {2, 3, 4, 5, 6, 7, 8, 9};
const int ledPinsBank2[] = {34, 35, 36, 37, 38, 39, 40, 41};
const int numLedsPerBank = 8;
int scannerPosition = 0;  // Tracks the currently lit LED (0-7)
int scannerDirection = 1; // Tracks direction: 1 = forward, -1 = reverse

/**
 * @brief Updates a bank of LEDs to match the state of a FlagManager instance.
 * @param flags A reference to the 8-bit FlagManager holding the desired LED states.
 * @param pins An array of pin numbers for the LED bank.
 */
void updateLedsFromFlags(FlagManager<uint8_t>& flags, const int pins[]) {
  for (int i = 0; i < numLedsPerBank; i++) {
    // Check if the flag at index 'i' is set
    bool ledState = flags.checkFlag(i);
    // Write the state to the corresponding LED pin
    digitalWrite(pins[i], ledState);
  }
}

/**
 * @brief Sets the flags in a FlagManager instance based on the bits of a byte.
 * @param flags A reference to the 8-bit FlagManager to modify.
 * @param data An 8-bit integer (byte) where each bit corresponds to a flag.
 */
void setFlagsFromByte(FlagManager<uint8_t>& flags, uint8_t data) {
  for (int i = 0; i < numLedsPerBank; i++) {
    // Check if the i-th bit of the data byte is 1
    if (bitRead(data, i)) {
      flags.setFlag(i); // If so, set the corresponding flag
    } else {
      flags.clearFlag(i); // Otherwise, clear it
    }
  }
}

//===========================================================
//  SETUP
//===========================================================
void setup() {
  Serial.begin(115200);
  
  // Use a floating analog pin to get a good random seed
  randomSeed(analogRead(A0)); 

  // Set all LED pins to OUTPUT
  for (int i = 0; i < numLedsPerBank; i++) {
    pinMode(ledPinsBank1[i], OUTPUT);
    pinMode(ledPinsBank2[i], OUTPUT);
  }
  Serial.println("LED banks configured. Starting random patterns...");
}


//===========================================================
//  LOOP - Cylon Scanner Effect
//===========================================================
void loop() {
  // 1. Calculate the LED pattern.
  //    The expression (1 << scannerPosition) creates a byte with only one bit
  //    set. For example, if scannerPosition is 2, the value is 00000100.
  uint8_t scannerPattern = 1 << scannerPosition;

  // 2. Set the flags for both banks using the same pattern.
  setFlagsFromByte(ledBank1Flags, scannerPattern);
  setFlagsFromByte(ledBank2Flags, scannerPattern);

  // 3. Update the physical LEDs to show the new pattern.
  updateLedsFromFlags(ledBank1Flags, ledPinsBank1);
  updateLedsFromFlags(ledBank2Flags, ledPinsBank2);

  // 4. Move the scanner to the next position.
  scannerPosition += scannerDirection;

  // 5. Check if the scanner has reached either end. If so, reverse direction.
  if (scannerPosition == numLedsPerBank - 1 || scannerPosition == 0) {
    scannerDirection *= -1; // Invert the direction (1 becomes -1, -1 becomes 1)
  }

  // 6. Wait for a moment to control the speed of the scanner.
  delay(10);
}
