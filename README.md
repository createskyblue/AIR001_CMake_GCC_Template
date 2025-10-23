# AIR001 CMake GCC Template Project

[中文版本](README.CN.md)

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

## Remote Embedded Development with GitHub Codespaces

You can develop, compile, and debug this embedded project remotely using GitHub Codespaces. Follow these steps:

1. Prepare your repository with CMake configuration and install dependencies:
   ```bash
   sudo apt install cmake ninja-build && pip install pyocd
   ```

2. Upload the ARM GCC toolchain archive `arm-gnu-toolchain-14.3.rel1-x86_64-arm-none-eabi.tar.xz` to `/opt`

3. Extract the toolchain:
   ```bash
   tar -xf arm-gnu-toolchain-14.3.rel1-x86_64-arm-none-eabi.tar.xz
   ```

4. Edit environment variables by modifying `~/.bashrc`:
   ```bash
   nano ~/.bashrc
   ```
   
   Add the following lines at the end:
   ```bash
   # ARM GCC Toolchain
   export ARM_TOOLCHAIN_PATH="/opt/arm-gnu-toolchain-14.3.rel1-x86_64-arm-none-eabi"
   export PATH="$ARM_TOOLCHAIN_PATH/bin:$PATH"
   ```

5. Apply the environment variables:
   ```bash
   source ~/.bashrc
   ```

6. If the environment setup doesn't work in GitHub Codespaces, you may need to hardcode the toolchain path in `cmake/gcc-arm-none-eabi.cmake`:
   ```cmake
   set(TOOLCHAIN_PREFIX /opt/arm-gnu-toolchain-14.3.rel1-x86_64-arm-none-eabi/bin/arm-none-eabi-)
   ```

7. Configure and build the project:
   - Run "CMake: Configure" from the command palette
   - Build the project

8. Install pyocd on your local machine:
   ```bash
   pip install pyocd
   ```

9. Create a batch file `Remote GDB Server.bat` on your local machine with the following content:
   ```batch
   @echo off
   :start
   pyocd gdbserver -t Air001
   goto start
   ```

10. Run this batch file to start the GDB server

11. Use FRP to expose your GDB server to the internet

12. Modify your GitHub Codespaces `.vscode/launch.json` with the following configuration:
    ```json
    {
        "version": "0.2.0",
        "configurations": [
            {
                "name": "Cortex Debug",
                "cwd": "${workspaceFolder}",
                "request": "launch",
                "type": "cortex-debug",
                "runToEntryPoint": "main",
                "executable": "./build/Project.elf",
                "targetId": "Air001",
                "armToolchainPath": "/opt/arm-gnu-toolchain-14.3.rel1-x86_64-arm-none-eabi/bin",
                "servertype": "external",
                "gdbTarget": "<your_frp_ip>:<your_frp_port>"
            }
        ]
    }
    ```

13. You can now start debugging with F5