# AIR001 CMake GCC Template Project

This is a template project for the AIR001 chip using CMake and GCC toolchain. The project demonstrates basic functionalities including UART loopback and LED blinking.

## Features

- UART loopback functionality
- LED blinking (1Hz)
- CMake build system with GCC toolchain
- DMA support for UART

## Hardware Configuration

### LED
- Pin: PB1 (GPIO Pin 1 on Port B)

### UART
- USART1 is used for communication
- TX: PF1 (GPIO Pin 1 on Port F)
- RX: PF0 (GPIO Pin 0 on Port F)
- Baudrate: 115200

## Functionality

1. The LED connected to PB1 will blink at 1Hz (on for 250ms, off for 250ms)
2. UART loopback is implemented - any data received on USART1 RX will be echoed back on USART1 TX

## Building

To build this project, you need:
1. GCC ARM toolchain
2. CMake
3. Properly configured build environment

Use standard CMake build process to compile the project.