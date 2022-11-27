# Summary
The LVRM, BPSM, DDRM and RRM boards are DIN rail modules of the DINFox project. They embed the following features:
* LVRM: **relay** with configurable coil voltage and controlled by the MCU.
* BPSM: **backup power supply** for the DINFox system.
* DDRM: **DC-DC converter** with configurable output voltage and controlled by the MCU.
* RRM: **rectifier and regulator** with configurable output voltage and controlled by the MCU.
* Input voltage, output voltage and output current **measurements**.
* **RS485** commmunication.

# Hardware
The boards were designed on **Circuit Maker V2.0**. Hardware documentation and design files are available @
* https://circuitmaker.com/Projects/Details/Ludovic-Lesur/LVRMHW1-0
* https://circuitmaker.com/Projects/Details/Ludovic-Lesur/BPSMHW1-0
* https://circuitmaker.com/Projects/Details/Ludovic-Lesur/DDRMHW1-0
* https://circuitmaker.com/Projects/Details/Ludovic-Lesur/RRMHW1-0

# Embedded software

## Environment
The embedded software was developed under **Eclipse IDE** version 2019-06 (4.12.0) and **GNU MCU** plugin. The `script` folder contains Eclipse run/debug configuration files and **JLink** scripts to flash the MCU.

## Target
The boards are based on the **STM32L011F4U6** of the STMicroelectronics L0 family microcontrollers. Each hardware revision has a corresponding **build configuration** in the Eclipse project, which sets up the code for the selected target.

## Structure
The project is organized as follow:
* `inc` and `src`: **source code** split in 4 layers:
    * `registers`: MCU **registers** adress definition.
    * `peripherals`: internal MCU **peripherals** drivers.
    * `components`: external **components** drivers.
    * `applicative`: high-level **application** layers.
* `startup`: MCU **startup** code (from ARM).
* `linker`: MCU **linker** script (from ARM).
