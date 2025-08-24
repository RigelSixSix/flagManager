flagManager
A lightweight, efficient, and versatile C++ template library for managing up to 32 boolean states (flags) in a single integer, perfect for embedded systems and Arduino projects.

flagManager is designed to help developers write clean, fast, and memory-efficient non-blocking code. It is an ideal tool for coordinating tasks, managing complex state machines, handling interrupts safely, and creating compact data packets for MCU-to-MCU communication.

Key Features
Memory Efficient: Manages up to 32 states in just 4 bytes, a significant memory saving over a standard boolean array.

Type Flexible: Built as a C++ template, it can be instantiated with uint8_t, uint16_t, or uint32_t to match the exact number of flags you need (8, 16, or 32).

ISR-Safe: Provides volatile-correct functions, making it perfect for safely and quickly passing events from an Interrupt Service Routine (ISR) to the main loop().

Promotes Non-Blocking Code: An essential tool for creating responsive, event-driven applications without using delay().

High-Density Communication: Ideal for creating compact and efficient data payloads for MCU-to-MCU communication over I2C, SPI, or UART.

Getting Started
Installation
Arduino Library Manager: (Once published) Open the Arduino IDE, navigate to Sketch > Include Library > Manage Libraries... and search for "flagManager".

Manual Installation: Download the latest release from this repository. In the Arduino IDE, choose Sketch > Include Library > Add .ZIP Library... and select the downloaded file.

Basic Usage
Here is a simple example of how to use flagManager in a sketch.

#include "flagManager.h"
#include <cstdint>

// 1. Instantiate the FlagManager with your chosen integer type.
//    This creates a manager for up to 32 flags.
FlagManager<uint32_t> tasks;

// 2. Use an enum to make your code more readable.
enum TaskFlags {
  TASK_READ_SENSOR,   // Bit 0
  TASK_UPDATE_SCREEN, // Bit 1
  TASK_SEND_DATA      // Bit 2
};

void setup() {
  Serial.begin(115200);

  // 3. Set flags to signal that tasks need to be done.
  tasks.setFlag(TASK_READ_SENSOR);
  tasks.setFlag(TASK_UPDATE_SCREEN);
}

void loop() {
  // 4. Check flags to determine which actions to take.
  if (tasks.checkFlag(TASK_READ_SENSOR)) {
    Serial.println("Reading sensor...");
    // ... your sensor code here ...

    // 5. Clear the flag once the task is complete.
    tasks.clearFlag(TASK_READ_SENSOR);
  }

  if (tasks.checkFlag(TASK_UPDATE_SCREEN)) {
    Serial.println("Updating screen...");
    // ... your display code here ...
    tasks.clearFlag(TASK_UPDATE_SCREEN);
  }
}

API Reference
The flagManager class is a template and must be instantiated with an integer type, e.g., FlagManager<uint8_t> myFlags;.

Volatile-Safe (ISR-Safe) Functions
These functions are designed to be callable on a volatile FlagManager object, making them safe for use inside an Interrupt Service Routine.

void setFlag(int flagIndex)
: Sets a specific flag (bit position) to 1. This is an atomic operation.

void clearFlag(int flagIndex)
: Clears a specific flag to 0. This is an atomic operation.

bool checkFlag(int flagIndex) const
: Returns true if the specified flag is 1, otherwise false.

void toggleFlag(int flagIndex)
: Inverts the state of a specific flag (0 becomes 1, 1 becomes 0). This is extremely useful in ISRs for acknowledging events.

T getRawFlags() const
: Returns the raw underlying integer (uint8_t, uint16_t, etc.) that holds all the flags.

void setAllFlags()
: Sets all flags in the register to 1.

void clearAllFlags()
: Clears all flags in the register to 0.

Non-Volatile Functions
These functions are not intended for use inside an ISR but are useful in the main loop() for debugging and general logic.

int getCapacity() const
: Returns the total number of flags the instance can hold (e.g., 8 for uint8_t, 32 for uint32_t).

String getFlagsString() const
: Returns a String object representing the flags in binary format (e.g., "00101101"). Very useful for printing to the Serial Monitor.

String getInverseFlagsString() const
: Returns a String of the inverted binary flags.

void setFlags(T value) : Sets the entire flag register to the provided integer value. This is very efficient for setting multiple flags at once from a predefined pattern or a received data packet.

Standalone Functions
int compareFlags(const FlagManager<T>& a, const FlagManager<T>& b)
: A standalone function that compares two FlagManager instances.

Returns 1 for an exact match.

Returns -1 for a partial match (some bits match, but not all).

Returns 0 for no match.

Use-Cases and Examples
The library includes six commented sketches in the examples folder to demonstrate its core functionality and advanced applications.

01_BasicFlags: Shows the fundamental operations of setting, checking, and clearing flags.

02_Cylon_Scanner: Demonstrates controlling hardware (LEDs) by using flags as a data model for a visual effect.

03_Pattern_Matching: Uses the compareFlags() function to find a match between a standard pattern and randomly generated ones.

04_ISR_Communication: An example showing how to safely use volatile flags to signal an event from a hardware interrupt to the main loop without data corruption.

05_System_State_Simulator: Implements a complex, multi-part system simulation where different FlagManager instances track the state of interdependent subsystems.

06_Finite_State_Machine: A more advanced version of the system simulator, refactored into a robust and scalable Finite State Machine (FSM) that uses a single FlagManager to hold the entire system state.

High-Density Data Payloads
One of the most powerful applications of flagManager is for MCU-to-MCU communication. When sending data over I2C, SPI, or a serial link, every byte counts. Instead of sending a string like "PUMP_ON,FAN_OFF,VALVE_OPEN" (26 bytes), you can represent the same information in a single byte.

The sender can set flags corresponding to the system state and send the single raw integer using getRawFlags().

// Sender MCU
FlagManager<uint8_t> commands;
commands.setFlag(CMD_PUMP_ON);    // bit 0
commands.setFlag(CMD_VALVE_OPEN); // bit 2
uint8_t payload = commands.getRawFlags(); // payload is B00000101
Wire.write(payload);

The receiver can read the byte and load it directly into its own flagManager instance using setFlags() to instantly decode all the commands.

// Receiver MCU
uint8_t receivedPayload = Wire.read(); // received B00000101
FlagManager<uint8_t> receivedCommands;
receivedCommands.setFlags(receivedPayload);

if (receivedCommands.checkFlag(CMD_PUMP_ON)) {
  // Turn the pump on...
}

Contributing
Contributions are welcome! If you have an idea for a new feature or have found a bug, please feel free to fork the repository, make your changes, and submit a pull request.

License
This project is licensed under the MIT License - see the LICENSE.md file for details.