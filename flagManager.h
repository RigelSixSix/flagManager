//===========================================================
//  flagManager.h (Universal)
//  Provides a fast and efficient means of setting 
//  and interacting with flags (bits) per instance.
//  Use to coordinate tasks, states and events between modules.

//  This templated version works with uint8_t, uint16_t, 
//  uint32_t.
//  The functions which can change the value of a bit are
//  declared volatile so they can be used in an ISR context.
//  
//  setFlag() can be used anywhere in your program (non-blocking)
//  clearFlag() can be used anywhere in your program (non-blocking)
//  checkFlag() is used to test the status of a flag, etc
//  Use FlagManager<uint8_t>, <uint16_t> or uint32_t to 
//  set the number of bits (flags).

//  Don Gibson
//  Greybeard Precision
//  Vancouver, Canada
//  V2.1.4 August 2025
//
//===========================================================

#ifndef flagManager_h
#define flagManager_h

#include <Arduino.h>

template <typename T>
class FlagManager {
public:
  // Constructor, initializes flags to 0
  FlagManager() : flags(0) {}

  // --- Volatile-Safe (ISR-Safe) Functions ---

  T getRawFlags() volatile const {
    return flags;
  }

  void setFlag(int flagIndex) volatile {
    if (flagIndex >= 0 && flagIndex < numFlags) {
      flags |= ((T)1 << flagIndex);
    }
  }

  void clearFlag(int flagIndex) volatile {
    if (flagIndex >= 0 && flagIndex < numFlags) {
      flags &= ~((T)1 << flagIndex);
    }
  }

  bool checkFlag(int flagIndex) volatile const {
    if (flagIndex >= 0 && flagIndex < numFlags) {
      return (flags & ((T)1 << flagIndex)) != 0;
    }
    return false;
  }

  void clearAllFlags() volatile {
    flags = 0;
  }

  void setAllFlags() volatile {
    flags = ~((T)0);
  }
  
  void toggleFlag(int flagIndex) volatile {
    if (flagIndex >= 0 && flagIndex < numFlags) {
      flags ^= ((T)1 << flagIndex);
    }
  }

  // --- Non-Volatile Functions ---

  /**
   * @brief Sets the entire flag register to the provided integer value.
   * @param value The integer value to load into the flags. Note it is
   * called setFlags() to set the entire integer vs. setFlag() which sets 1 bit
   */
  void setFlags(T value) {
    flags = value;
  }

  int getCapacity() const {
    return numFlags;
  }

  String getFlagsString() const {
    String result = "";
    T flags_copy = flags; // Create a non-volatile copy
    for (int i = numFlags - 1; i >= 0; i--) {
      result += (flags_copy & ((T)1 << i)) ? '1' : '0';
    }
    return result;
  }

  String getInverseFlagsString() const {
    String result = "";
    T flags_copy = flags; // Create a non-volatile copy
    for (int i = numFlags - 1; i >= 0; i--) {
      result += (flags_copy & ((T)1 << i)) ? '0' : '1';
    }
    return result;
  }

private:
  volatile T flags; // The internal data storage of placeholder type T
  
  static const int numFlags = sizeof(T) * 8; 
};

// Standalone compareFlags function
template <typename T>
int compareFlags(const FlagManager<T>& a, const FlagManager<T>& b) {
  T a_flags = a.getRawFlags();
  T b_flags = b.getRawFlags();

  if (a_flags == b_flags) {
    return 1; // Exact match
  }
  if ((a_flags & b_flags) != 0) {
    return -1; // Partial match
  }
  return 0; // No match
}

#endif
