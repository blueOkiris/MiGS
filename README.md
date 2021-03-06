# MiGS (Microcontroller Game System)

## Description

A cheap and easy to program game console based on multiple microcontrollers interacting.

## Building

- Dependencies:
  + Linux System
  + gcc
  + make
  + cmake
  + git
  + gcc-arm-none-eabi
  + libnewlib-arm-none-eabi

To flash the programmer run `make upload-pgrmr PORT=<insert port>`

To build the menu program run `make MigsMenu.ino.hex`

To build the gpu program run `make MigsGpu.uf2`

To flash the ErrorReceiver program, use the Arduino IDE

## System Design

3 Parts: Programming MCU, Logic MCU, and Graphics MCU

__Graphics MCU:__
- Raspberry Pi PICO
- Outputs [sprite-based system to VGA](https://www.youtube.com/watch?v=RmPWcsvGSyk) (which gets [adapted to HDMI](https://www.amazon.com/Monitor-Connector-VENTION-Adapter-Computer/dp/B08GZ159FJ/ref=sr_1_6?crid=1TODLD3WMDJ1C&keywords=vga+to+hdmi&qid=1645044383&sprefix=vga+to+hdm%2Caps%2C127&sr=8-6))
- Learns what to draw via communication with Logic MCU

__Logic MCU:__
- Actually what people program for
- Sends graphics info to Graphics MCU to be drawn since it's underpowered
- Main logic for games
- ATMega328p (i.e. essentially and Arduino UNO)
- Processes controller inputs

__Programming MCU:__
- [Programs the Logic MCU from SD card](https://baldwisdom.com/bootdrive/)
- Provides access to sd card data for the Logic MCU
- Processes controller inputs to actually select a game (menu program "fake")

__Error Receiver__
- Serial connection to programmer for debugging
- Requires enabling debug flag in programmer's .hpp files and connecting programmer 5, 4 to this device's 4, 5
