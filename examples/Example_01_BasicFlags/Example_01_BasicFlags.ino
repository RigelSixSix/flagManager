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
//  FlagManager Example: 01_BasicFlags
//  Demonstrates the basic functions of the FlagManager library,
//  including creating instances, setting, clearing, and
//  checking flags.
//===========================================================
#include "flagManager.h"
#include <cstdint>

// Create two 32-bit flag manager instances.
// The template <uint32_t> specifies that each instance can
// hold up to 32 individual flags (0-31).
FlagManager<uint32_t> flagsOne;
FlagManager<uint32_t> flagsTwo;

// Define an enum to make flag indices more readable and
// prevent magic numbers in the code.
enum MyFlags {
  FLAG_A = 5,
  FLAG_B = 10,
  FLAG_C = 15,
  FLAG_D = 20
};

void setup() {
  /**
   * @brief Initializes serial communication and demonstrates
   * the core flag operations in a sequence.
   */
  Serial.begin(115200);
  while (!Serial); // Wait for Serial port to connect

  Serial.println("--- FlagManager Basic Demo ---");

  // 1. Set individual flags using the enum values.
  flagsOne.setFlag(FLAG_A); // Sets bit 5
  flagsOne.setFlag(FLAG_B); // Sets bit 10
  flagsOne.setFlag(FLAG_C); // Sets bit 15
  flagsTwo.setFlag(FLAG_D); // Sets bit 20

  // 2. Clear a specific flag.
  flagsTwo.clearFlag(FLAG_D);

  // 3. Read and print the flags as a binary string.
  Serial.print("Initial flagsOne:         ");
  Serial.println(flagsOne.getFlagsString());
  Serial.print("Initial flagsTwo:         ");
  Serial.println(flagsTwo.getFlagsString());

  // 4. Print the inverse of the flags.
  Serial.print("Inverse of flagsOne:      ");
  Serial.println(flagsOne.getInverseFlagsString());

  // 5. Set all flags to 1.
  flagsOne.setAllFlags();
  Serial.print("flagsOne after setAllFlags: ");
  Serial.println(flagsOne.getFlagsString());

  // 6. Clear all flags back to 0.
  flagsOne.clearAllFlags();
  Serial.print("flagsOne after clearAllFlags: ");
  Serial.println(flagsOne.getFlagsString());
  Serial.println("---------------------------------");
}

void loop() {
  /**
   * @brief Continuously sets random flags and checks specific
   * flags to demonstrate real-time monitoring.
   */
  // Set a random flag between 0 and 31 in each instance.
  uint8_t randomBit1 = random(0, 32);
  uint8_t randomBit2 = random(0, 32);
  flagsOne.setFlag(randomBit1);
  flagsTwo.setFlag(randomBit2);

  Serial.print("Set random bits: ");
  Serial.print(randomBit1);
  Serial.print(" in flagsOne, ");
  Serial.print(randomBit2);
  Serial.println(" in flagsTwo.");

  // Check the value of a specific bit and take action.
  if (flagsOne.checkFlag(FLAG_A)) {
    Serial.println(">> Action: Bit 5 was set in flagsOne!");
    // Once acted upon, clear the flag.
    flagsOne.clearFlag(FLAG_A);
  }

  if (flagsTwo.checkFlag(FLAG_C)) {
    Serial.println(">> Action: Bit 15 was set in flagsTwo!");
    flagsTwo.clearFlag(FLAG_C);
  }

  // Print the current state of the flags for this loop iteration.
  Serial.print("Current flagsOne: ");
  Serial.println(flagsOne.getFlagsString());
  Serial.print("Current flagsTwo: ");
  Serial.println(flagsTwo.getFlagsString());
  Serial.println();

  delay(2000); // Delay for demonstration purposes.
}
