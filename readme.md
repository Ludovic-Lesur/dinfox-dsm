# Summary
The LVRM board is a DIN rail module embedding the following features:
* **Relay** with configurable coil voltage and controlled by the MCU.
* Input voltage, output voltage and output current **measurements**.
* **RS485** commmunication.

# Hardware
The board was designed on **Circuit Maker V2.0**. Hardware documentation and design files are available @ https://circuitmaker.com/Projects/Details/Ludovic-Lesur/LVRMHW1-0

# Embedded software

## Environment
The embedded software was developed under **Eclipse IDE** version 2019-06 (4.12.0) and **GNU MCU** plugin. The `script` folder contains Eclipse run/debug configuration files and **JLink** scripts to flash the MCU.

## Target
The LVRM board is based on the **STM32L011F3U6** of the STMicroelectronics L0 family microcontrollers. Each hardware revision has a corresponding **build configuration** in the Eclipse project, which sets up the code for the selected target.

## Structure
The project is organized as follow:
* `inc` and `src`: **source code** split in 4 layers:
    * `registers`: MCU **registers** adress definition.
    * `peripherals`: internal MCU **peripherals** drivers.
    * `components`: external **components** drivers.
    * `applicative`: high-level **application** layers.
* `startup`: MCU **startup** code (from ARM).
* `linker`: MCU **linker** script (from ARM).
