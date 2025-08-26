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

#ifndef flagManager_u_h
#define flagManager_u_h

#include <Arduino.h>

template <typename T> // Make the class a template with placeholder type T
class FlagManager {
public:
  // Constructor, initializes flags to 0
  FlagManager() : flags(0) {}

  // --- Volatile-Safe Functions ---
  // These are designed to be callable on a volatile object, such as in an ISR.

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
  // These are optimized for performance and are NOT intended for use in an ISR.

  int getCapacity() const {
    return numFlags;
  }

  String getFlagsString() const {
    String result = "";
    T flags_copy = flags; // Create a non-volatile copy for high performance
    for (int i = numFlags - 1; i >= 0; i--) {
      result += (flags_copy & ((T)1 << i)) ? '1' : '0';
    }
    return result;
  }

  String getInverseFlagsString() const {
    String result = "";
    T flags_copy = flags; // Create a non-volatile copy for high performance
    for (int i = numFlags - 1; i >= 0; i--) {
      result += (flags_copy & ((T)1 << i)) ? '0' : '1';
    }
    return result;
  }

private:
  T flags; // The internal data storage of placeholder type T
  
  static const int numFlags = sizeof(T) * 8; 
};

//======================================================================
//  STANDALONE HELPER FUNCTION
//  Compares two FlagManager instances.
//  Returns:
//   1: All bits are identical (exact match)
//  -1: At least one bit matches, but not all (partial match)
//   0: No bits match at all
//======================================================================
template <typename T>
int compareFlags(const FlagManager<T>& fm1, const FlagManager<T>& fm2) {
  // getRawFlags() must be volatile const to be called on a potentially volatile object
  // This allows the function to be used in an ISR context.
  // 
  // The flags are stored in a type T, which can be uint8_t, uint16_t, or uint32_t.
  // The function compares the flags of both instances and returns:
  // 1 if all bits are identical,
  // -1 if at least one bit matches but not all, (e.g. test if any pumps are running)
  // 0 if no bits match at all.
  // This is useful for checking the state of flags across different 
  // FlagManager instances, or when comparing a set of flags to a known condition
  
  T flags1 = fm1.getRawFlags();
  T flags2 = fm2.getRawFlags();

  // Case 1: All bits are identical
  if (flags1 == flags2) {
    return 1;
  }

  // Case 2: At least one bit matches, but not all
  if ((flags1 & flags2) != 0) {
    return -1;
  }

  // Case 3: No bits match at all
  return 0;
}

#endif
