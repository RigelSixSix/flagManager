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
//  V2.1.3 August 2025
//
//  FlagManager Example: 03_Pattern_Matching
//  Demonstrates how to use a compareFlags() function to
//  check for matching bits between two FlagManager instances.
//===========================================================
#include "flagManager.h"
#include <cstdint>

// --- Flag Managers & Standard Pattern ---
FlagManager<uint8_t> standardPatternFlags;
FlagManager<uint8_t> randomPatternFlags;

// The pattern to match against. 0b01010101 is the common test pattern (0x55 in hex).
const uint8_t standardPattern = 0b01010101;

// --- Hardware Definitions ---
const int ledPinsBank1[] = {2, 3, 4, 5, 6, 7, 8, 9};       // Bank 1 shows match status
const int ledPinsBank2[] = {34, 35, 36, 37, 38, 39, 40, 41}; // Bank 2 shows the random pattern
const int numLedsPerBank = 8;

/**
 * @brief Sets the flags in a FlagManager instance from the bits of a byte.
 * @param flags A reference to the 8-bit FlagManager to modify.
 * @param data An 8-bit integer (byte) where each bit corresponds to a flag.
 * since the actial variable is an int(8|16|32), data can also be a numeric value
 */
void setFlagsFromByte(FlagManager<uint8_t>& flags, uint8_t data) {
  // First, reset all flags to a known state (0) to ensure a clean slate.
  flags.clearAllFlags();

  // Now, iterate through the byte and set only the flags that should be 1.
  for (int i = 0; i < numLedsPerBank; i++) {
    if (bitRead(data, i)) {
      flags.setFlag(i);
    }
  }
}

/**
 * @brief Updates a bank of LEDs to match the state of a FlagManager instance.
 * @param flags A reference to the 8-bit FlagManager holding the desired LED states.
 * @param pins An array of pin numbers for the LED bank.
 */
void updateLedsFromFlags(const FlagManager<uint8_t>& flags, const int pins[]) {
  for (int i = 0; i < numLedsPerBank; i++) {
    bool ledState = flags.checkFlag(i);
    digitalWrite(pins[i], ledState);
  }
}

/**
 * @brief Turns off all LEDs in bank 1.
 */
void indicateNoMatch() {
  for (int i = 0; i < numLedsPerBank; i++) {
    digitalWrite(ledPinsBank1[i], LOW);
  }
}

/**
 * @brief Indicates a partial match by turning on the upper half of LED bank 1.
 */
void indicatePartialMatch() {
  indicateNoMatch(); // Turn off all bank 1 LEDs first for a clean signal.
  for (int i = 4; i < numLedsPerBank; i++) {             // turn on 4 to 7
    digitalWrite(ledPinsBank1[i], HIGH);
  }
}

/**
 * @brief Indicates an exact match by turning on all of LED bank 1.
 */
void indicateExactMatch() {
  for (int i = 0; i < numLedsPerBank; i++) {
    digitalWrite(ledPinsBank1[i], HIGH);
  }
}

void setup() {
  /**
   * @brief Initializes hardware and sets up the standard pattern for comparison.
   */
  Serial.begin(115200);
  while (!Serial);
  randomSeed(analogRead(A0));

  // Set all LED pins to OUTPUT mode.
  for (int i = 0; i < numLedsPerBank; i++) {
    pinMode(ledPinsBank1[i], OUTPUT);
    pinMode(ledPinsBank2[i], OUTPUT);
  }

  // Load the standard pattern into its dedicated flag instance.
  setFlagsFromByte(standardPatternFlags, standardPattern);

  Serial.println("--- FlagManager Pattern Matcher Demo ---");
  Serial.print("Standard Pattern to Match: ");
  Serial.println(standardPatternFlags.getFlagsString());
  Serial.println("Starting random search...");
}

void loop() {
  /**
   * @brief Continuously generates random patterns and compares them to the standard.
   */
  // 1. Generate a new random 8-bit number and load it into the flags.
  uint8_t randomNumber = random(0, 256);
  setFlagsFromByte(randomPatternFlags, randomNumber);

  // Immediately display the generated random pattern on LED Bank 2.
  updateLedsFromFlags(randomPatternFlags, ledPinsBank2);

  // 2. Compare the random pattern against the standard pattern.
  //    This assumes a standalone `compareFlags` function exists in the library.
  //    - Returns 1 for an exact match.
  //    - Returns -1 for a partial match (some bits match, but not all).
  //    - Returns 0 for no match.
  int result = compareFlags(standardPatternFlags, randomPatternFlags);

  // 3. Take action based on the comparison result, updating LED Bank 1.
  switch (result) {
    case 1: // Exact Match
      indicateExactMatch();
      Serial.print(">>> EXACT MATCH FOUND! Pattern: ");
      Serial.println(randomPatternFlags.getFlagsString());
      delay(2000); // Pause to show the match.
      break;

    case -1: // Partial Match
      indicatePartialMatch();
      break;

    case 0: // No Match
      indicateNoMatch();
      break;
  }

  delay(50); // Controls the speed of the random test.
}
