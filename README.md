# Description

This repository contains the embedded software of the **DinFox slaves modules**:

* LVRM: **relay** with configurable coil voltage and controlled by the MCU.
* BPSM: **backup power supply** for the DINFox system.
* DDRM: **DC-DC converter** with configurable output voltage and controlled by the MCU.
* UHFM: **Sub-GHz modem** for radio monitoring and remote control.
* RRM: **rectifier and regulator** with configurable output voltage and controlled by the MCU.
* SM: **sensors module** with embedded temperature/humidity sensor, 4 analog inputs, 4 digital I/Os and external shield support (with I2C and I/Os).
* GPSM: **GPS module** with active antenna support.
* MPMCM: **Mains monitoring and controller module** with 4 independent channels (true RMS voltage, true RMS current, active power, apparent power, power factor and frequency), real time control of 4 loads (triac) and Linky TIC interface.
* BCM: **battery charger** with configurable charge voltage and current.
* Analog **measurements** such as input voltage, output voltage and output current.
* **RS485** communication.

# Hardware

The boards were designed on **Circuit Maker V2.0**. Below is the list of hardware revisions:

| Hardware revision | Description | `cmake_board` | `cmake_hw_version` | Status |
|:---:|:---:|:---:|:---:|:---:|
| [LVRM HW1.0](https://365.altium.com/files/10D8C121-B324-4AC0-90B1-A0BFFB7E4713) | Initial version with monostable relay. | `LVRM` | `HW1_0` | :white_check_mark: |
| [LVRM HW2.0](https://365.altium.com/files/5F3B7EA9-DD07-4C07-B750-9D2D3ABDA776) | Initial version with bistable relay. | `LVRM` | `HW2_0` | :white_check_mark: |
| [BPSM HW1.0](https://365.altium.com/files/BAC116F3-F512-4102-9D47-53DF0FB6E9C0) | Initial version. | `BPSM` | `HW1_0` | :white_check_mark: |
| [DDRM HW1.0](https://365.altium.com/files/1BA47FD8-3599-4BA0-8A3B-857EFF1E8E58) | Initial version. | `DDRM` | `HW1_0` | :white_check_mark: |
| [UHFM HW1.0](https://365.altium.com/files/C3D2D8A0-D05C-40FD-AE3A-D0FEBA8A509F) | Initial version. | `UHFM` | `HW1_0` | :x: |
| [UHFM HW2.0](https://365.altium.com/files/022E57D6-9B88-414B-A520-93281961BF8E) | New radio front-end with extended features. | `UHFM` | `HW2_0` | :white_check_mark: |
| [RRM HW1.0](https://365.altium.com/files/F33BFE95-AA3E-4890-B685-3A09A36AE775) | Initial version. | `RRM` | `HW1_0` | :white_check_mark: |
| [SM HW1.0](https://365.altium.com/files/73597AC1-81FF-471F-A80B-41D71904A039) | Initial version. | `SM` | `HW1_0` | :white_check_mark: |
| [GPSM HW1.0](https://365.altium.com/files/86BC5960-7B01-45BE-B7A5-BD8ADBCE5E8D) | Initial version. | `GPSM` | `HW1_0` | :white_check_mark: |
| [MPMCM HW1.0](https://365.altium.com/files/DD635FDD-1D00-456C-9219-78701675DC01) | Initial version. | `MPMCM` | `HW1_0` | :white_check_mark: |
| [BCM HW1.0](https://365.altium.com/files/05D7821F-F16C-4190-8AAC-8EBAEC7074C2) | Initial version. | `BCM` | `HW1_0` | :white_check_mark: |

# Embedded software

## Environment

The firmware is developed under **Eclipse IDE** and **GNU MCU** plugin. The `script` folder contains Eclipse run/debug configuration files and **JLink** scripts to flash the MCU.

## Target

The boards are based on various **STM32L0x1** and **STM32G4x1** microcontrollers of the STMicroelectronics L0/G4 families (see table below). Each hardware revision has a corresponding **build configuration** in the Eclipse project, which sets up the code for the selected board.

## Architecture

<p align="center">
<img src="https://github.com/Ludovic-Lesur/dinfox-doc/blob/master/images/dsm-sw-architecture.drawio.png" width="600"/>
</p>

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
    * `cli` : **AT commands** implementation.
    * `digital` : High level **digital I/O** driver (SM only).
    * `gps` : High level **GPS** driver (GPSM only).
    * `node` : **UNA** nodes interface implementation.
    * `power` : Board **power tree** manager.
    * `sigfox` : **Sigfox EP_LIB** and **ADDON_RFP** submodules and low level implementation (UHFM only).
* `application` : Main **application**.

## Sigfox library

The **UHFM** board uses **Sigfox technology** to perform the system remote monitoring (and light remote control). The project is based on the [Sigfox end-point open source library](https://github.com/sigfox-tech-radio/sigfox-ep-lib) which is embedded as a **Git submodule**.

## Build

The project can be compiled by command line with `cmake`.

```bash
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE="script/cmake-arm-none-eabi/toolchain.cmake" \
      -DTOOLCHAIN_PATH="<arm_none_eabi_gcc_path>" \
      -DDSM_BOARD="<cmake_board>" \
      -DDSM_HW_VERSION="<cmake_hw_version>" \
      -DDSM_NVM_FACTORY_RESET=OFF \
      -DDSM_NODE_ADDRESS=0x7F \
      -G "Unix Makefiles" ..
make all
```

## Flash

### Boards parameters

| Hardware revision | MCU | Programming connector |
|:---:|:---:|:---:|
| [LVRM HW1.0](https://365.altium.com/files/10D8C121-B324-4AC0-90B1-A0BFFB7E4713) | STM32L011F4U6 | P6 |
| [LVRM HW2.0](https://365.altium.com/files/5F3B7EA9-DD07-4C07-B750-9D2D3ABDA776) | STM32L031G6U6 | P4 |
| [BPSM HW1.0](https://365.altium.com/files/BAC116F3-F512-4102-9D47-53DF0FB6E9C0) | STM32L011F4U6 | P6 |
| [DDRM HW1.0](https://365.altium.com/files/1BA47FD8-3599-4BA0-8A3B-857EFF1E8E58) | STM32L011F4U6 | P6 |
| [UHFM HW1.0](https://365.altium.com/files/C3D2D8A0-D05C-40FD-AE3A-D0FEBA8A509F) | STM32L041K6U6 | P4 |
| [UHFM HW2.0](https://365.altium.com/files/022E57D6-9B88-414B-A520-93281961BF8E) | STM32L051C8U6 | P3 |
| [RRM HW1.0](https://365.altium.com/files/F33BFE95-AA3E-4890-B685-3A09A36AE775) | STM32L011F4U6 | P5 |
| [SM HW1.0](https://365.altium.com/files/73597AC1-81FF-471F-A80B-41D71904A039) | STM32L031G6U6 | P4 |
| [GPSM HW1.0](https://365.altium.com/files/86BC5960-7B01-45BE-B7A5-BD8ADBCE5E8D) | STM32L031G6U6 | P6 |
| [MPMCM HW1.0](https://365.altium.com/files/DD635FDD-1D00-456C-9219-78701675DC01) | STM32G441CBT6 | P6 |
| [BCM HW1.0](https://365.altium.com/files/05D7821F-F16C-4190-8AAC-8EBAEC7074C2) | STM32L011G4U6 | P4 |

### Preparation

* **Build** the desired version (with IDE or `cmake`) or **download** a specific [firmware release](https://github.com/Ludovic-Lesur/dinfox-dsm/releases) (expand the `Assets` menu, download the corresponding artifact and extract the binary files from the `zip`).
* Connect the flashing tool to the **programming connector** (see previous table) located in the corner of the PCB (standard SWD pinout).

### ST-Link on Nucleo board

* Make sure that the ST-LINK/NUCLEO jumpers (generally designated by **CN2**) are not fitted, in order to **select the external programming connector** instead of the internal MCU.
* An **MSC disk** named `NODE_XXXXXX` should be mounted by the system after USB plugging. If not, download the [ST Cube Programmer](https://www.st.com/en/development-tools/stm32cubeprog.html) software which will install the required drivers. If the MSC disk is still not mounted, follow the ST-Link probe procedure thereafter.
* **Copy/paste** or **click/drop** the `bin` file into the disk.

### ST-Link probe

* Download the [ST Cube Programmer](https://www.st.com/en/development-tools/stm32cubeprog.html) software.
* Launch the software (it might be necessary to run it as **root** or to install specific **USB rules** for the probe to be recognized).
* In the right panel, select `ST-LINK` and click `Connect`.
* Click on the `Open file` tab and select the `hex` file to flash.
* Click on the `Download` button.
* Perform a **memory check** with the `Verify` button located under the `Download` button menu.
* If the operation completed successfully, click on `Disconnect` in the right panel.

### Segger J-Link probe

* Download the [Segger J-Link](https://www.segger.com/downloads/jlink/) software.
* Launch the `JFlashLite` tool.
* Select the **target device** (see previous table, use MCU name without the last 2 digits), set target interface to **SWD**, speed to **4000kHz** and click `OK`.
* Open the `hex` file to flash.
* Click on the `Program Device` button.

### Final steps

* Check on the platform if the board has properly rebooted with the **expected firmware version**.
