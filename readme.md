# Summary
The LVRM, BPSM, DDRM, UHFM, RRM, SM and GPSM boards are DIN rail modules of the DINFox project. They embed the following features:
* LVRM: **relay** with configurable coil voltage and controlled by the MCU.
* BPSM: **backup power supply** for the DINFox system.
* DDRM: **DC-DC converter** with configurable output voltage and controlled by the MCU.
* UHFM: **433 / 868 MHz modem** for radio monitoring and remote control.
* RRM: **rectifier and regulator** with configurable output voltage and controlled by the MCU.
* SM: **sensors module** with embedded temperature/humidity sensor, 4 analog inputs, 4 digital I/Os and external shield support (with I2C and I/Os).
* GPSM: **GPS module** with active antenna support.
* Analog **measurements** such as input voltage, output voltage and output current.
* **RS485** communication.

# Hardware
The boards were designed on **Circuit Maker V2.0**. Hardware documentation and design files are available @
* https://circuitmaker.com/Projects/Details/Ludovic-Lesur/LVRMHW1-0
* https://circuitmaker.com/Projects/Details/Ludovic-Lesur/LVRMHW2-0
* https://circuitmaker.com/Projects/Details/Ludovic-Lesur/BPSMHW1-0
* https://circuitmaker.com/Projects/Details/Ludovic-Lesur/DDRMHW1-0
* https://circuitmaker.com/Projects/Details/Ludovic-Lesur/RRMHW1-0
* https://circuitmaker.com/Projects/Details/Ludovic-Lesur/SMHW1-0
* https://circuitmaker.com/Projects/Details/Ludovic-Lesur/GPSMHW1-0
* https://circuitmaker.com/Projects/Details/Ludovic-Lesur/UHFMHW1-0

# Embedded software

## Environment
The embedded software was developed under **Eclipse IDE** version 2019-06 (4.12.0) and **GNU MCU** plugin. The `script` folder contains Eclipse run/debug configuration files and **JLink** scripts to flash the MCU.

## Target
The boards are based on the **STM32L011F4U6**, **STM32L031G6U6** and **STM32L041K6U6** of the STMicroelectronics L0 family microcontrollers. Each hardware revision has a corresponding **build configuration** in the Eclipse project, which sets up the code for the selected target.

## Structure
The project is organized as follow:
* `inc` and `src`: **source code** split in 7 layers:
    * `registers`: MCU **registers** adress definition.
    * `peripherals`: internal MCU **peripherals** drivers.
    * `utils`: **utility** functions.
    * `components`: external **components** drivers.
    * `sigfox`: **Sigfox library** API and low level implementation.
    * `nodes` : Nodes **descriptors**.
    * `applicative`: high-level **application** layers.
* `startup`: MCU **startup** code (from ARM).
* `linker`: MCU **linker** script (from ARM).
