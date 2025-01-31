# Description

The **LVRM**, **BPSM**, **DDRM**, **UHFM**, **RRM**, **SM** and **GPSM** boards are DIN rail modules of the DINFox project. They embed the following features:

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

The boards were designed on **Circuit Maker V2.0**. Below is the list of hardware revisions:

| Hardware revision | Description | Status |
|:---:|:---:|:---:|
| [LVRM HW1.0](https://365.altium.com/files/10D8C121-B324-4AC0-90B1-A0BFFB7E4713) | Initial version with monostable relay. | :white_check_mark: |
| [LVRM HW2.0](https://365.altium.com/files/5F3B7EA9-DD07-4C07-B750-9D2D3ABDA776) | Initial version with bistable relay. | :white_check_mark: |

| Hardware revision | Description | Status |
|:---:|:---:|:---:|
| [BPSM HW1.0](https://365.altium.com/files/BAC116F3-F512-4102-9D47-53DF0FB6E9C0) | Initial version. | :white_check_mark: |

| Hardware revision | Description | Status |
|:---:|:---:|:---:|
| [DDRM HW1.0](https://365.altium.com/files/1BA47FD8-3599-4BA0-8A3B-857EFF1E8E58) | Initial version. | :white_check_mark: |

| Hardware revision | Description | Status |
|:---:|:---:|:---:|
| [UHFM HW1.0](https://365.altium.com/files/C3D2D8A0-D05C-40FD-AE3A-D0FEBA8A509F) | Initial version. | :white_check_mark: |

| Hardware revision | Description | Status |
|:---:|:---:|:---:|
| [RRM HW1.0](https://365.altium.com/files/F33BFE95-AA3E-4890-B685-3A09A36AE775) | Initial version. | :white_check_mark: |

| Hardware revision | Description | Status |
|:---:|:---:|:---:|
| [SM HW1.0](https://365.altium.com/files/73597AC1-81FF-471F-A80B-41D71904A039) | Initial version. | :white_check_mark: |

| Hardware revision | Description | Status |
|:---:|:---:|:---:|
| [GPSM HW1.0](https://365.altium.com/files/86BC5960-7B01-45BE-B7A5-BD8ADBCE5E8D) | Initial version. | :white_check_mark: |

# Embedded software

## Environment

The embedded software is developed under **Eclipse IDE** version 2024-09 (4.33.0) and **GNU MCU** plugin. The `script` folder contains Eclipse run/debug configuration files and **JLink** scripts to flash the MCU.

> [!WARNING]
> To compile any version under `sw4.0`, the `git_version.sh` script must be patched when `sscanf` function is called: the `SW` prefix must be replaced by `sw` since Git tags have been renamed in this way.

## Target

The boards are based on the **STM32L011F4U6**, **STM32L031G6U6** and **STM32L041K6U6** microcontrollers of the STMicroelectronics L0 family. Each hardware revision has a corresponding **build configuration** in the Eclipse project, which sets up the code for the selected board.

## Structure

The project is organized as follow:

* `drivers` :
    * `device` : MCU **startup** code and **linker** script.
    * `registers` : MCU **registers** address definition.
    * `peripherals` : internal MCU **peripherals** drivers.
    * `mac` : **medium access control** driver.
    * `components` : external **components** drivers.
    * `utils` : **utility** functions.
* `middleware` :
    * `analog` : High level **analog measurements** driver.
    * `node` : **UNA** nodes interface implementation.
    * `power` : Board **power tree** manager.
    * `sigfox` : **Sigfox EP_LIB** and **ADDON_RFP** submodules and low level implementation.
* `application` : Main **application**.

## Sigfox library

The **UHFM** board uses **Sigfox technology** to perform the system remote monitoring (and light remote control). The project is based on the [Sigfox end-point open source library](https://github.com/sigfox-tech-radio/sigfox-ep-lib) which is embedded as a **Git submodule**.
