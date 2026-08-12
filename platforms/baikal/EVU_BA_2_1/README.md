# Description

EVU-BA-2.1 development board (EValUation BAse board, version 2.1) is designed to simplify the development and debugging of user projects.\
This BSP is intended to support the EVU-BA-2.1 board and assist in developing new software projects based on it.

# Board specification

- BE-U1000 MCU
- 16 MB QSPI flash memory chip with XiP mode support
- Boot mode selection (bootstrap pins pull-up/pull-down jumpers)
- USB Type-C port related to the MCU USB interface
- Reset button and "power on" LED
- All MCU pins are available on the pin headers
- Arduino Uno-type hat connector
- USB Type-C interface to the integrated FTDI-based debugger and serial communication
- Support for different types of power sources (USB port, external 5V source, or an external power supply with a wide voltage range)
- User-controlled push button and LED
- External JTAG connection ability (10-Pin 0.05" micro JTAG connector)

Get more info in the board's documentation.

# BSP composition

Includes functions and definitions for board components:

- User LED
- User button
- Serial interface (the port connected to the FTDI UART-USB on-board converter)
- UART (the port used for demonstration of UART interface functionality, by default the same as serial)
