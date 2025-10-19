# pllatinum-test-board

Firmware and command line interface for a microwave synthesizer test board based on the **LMX2592**.

## Overview

This project provides firmware for a custom RF test board using the Texas Instruments **LMX2592** microwave frequency synthesizer. The board is controllable via a simple UART serial command-line interface and allows full configuration of the PLL including:

* Frequency tuning (20 MHz → 9.8 GHz)
* Output power control (integer scale 0 → 47)
* Enabling/disabling RF output channels
* Register dumping (debugging)
* Rebooting MCU into USB bootloader mode for reprogramming

Internally, this firmware targets the RP2040 (Raspberry Pi Pico or equivalent) using the Pico SDK, communicating with the LMX2592 over SPI.

---

## Hardware Summary

| Component                | Description                        |
| ------------------------ | ---------------------------------- |
| PLL IC                   | LMX2592 Microwave Synthesizer      |
| MCU                      | RP2040 (Pico SDK)                  |
| Reference Clock          | 48 MHz TCXO (0.5 ppm)              |
| PFD Clock                | 120 MHz (REF * 5 / 2)              |
| Frequency Range          | 20 MHz → 9800 MHz                  |
| Fundamental (no doubler) | up to ~7100 MHz                    |
| Output Channels          | RF1 / RF2 independently switchable |
| USB Power Draw           | ~400 mA                            |

---

## Using the CLI

Connect a serial terminal (e.g., PuTTY, screen, minicom) to the board at the default UART speed (e.g. 115200 8N1).
After boot, input commands in the form shown below.

### CLI Command Reference

| Command           | Arguments     | Description                           | Example           |
| ----------------- | ------------- | ------------------------------------- | ----------------- |
| `-help`           | *(none)*      | Prints usage help                     | `-help`           |
| `-f`              | `<float MHz>` | Sets frequency in MHz (20.0 → 9800.0) | `-f 2400.5`       |
| `-p`              | `0–47`        | Sets RF power level                   | `-p 15`           |
| `-rf1`            | `on/off`      | Enables or disables RF channel 1      | `-rf1 on`         |
| `-rf2`            | `on/off`      | Enables or disables RF channel 2      | `-rf2 off`        |
| `-d`              | `hex/bin`     | Dumps LMX2592 registers               | `-d hex`          |
| `-reboot`         | *(none)*      | Reboot MCU to USB bootloader          | `-reboot`         |
| `-about`          | *(none)*      | Prints board metadata                 | `-about`          |

### Notes

* RF output is **off by default** — enable `-rf1 on` or `-rf2 on` after frequency set.
* Frequency specified in MHz; internally converted to Hz.
* Lock time is measured and reported.
* Power limits enforced: max 47.

---

## Repository Structure

| File             | Description                               |
| ---------------- | ----------------------------------------- |
| `main.cpp`       | CLI parsing, SPI init, PLL control loop   |
| `lmx2592.h/.cpp` | Driver for LMX2592 registers and controls |
| `CMakeLists.txt` | Pico SDK build definition                 |
| `.vscode/`       | Optional editor configs                   |

---


