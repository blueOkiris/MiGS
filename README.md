# MiGS (Microcontroller Game System)

## Description

A cheap and easy to program game console based on multiple microcontrollers interacting.

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