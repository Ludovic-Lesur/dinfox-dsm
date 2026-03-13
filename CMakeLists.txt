#
# CMakeLists.txt
#
#  Created on: 11 mar. 2026
#      Author: Ludo
#

# Minimum CMake version.
cmake_minimum_required(VERSION 3.23)

# Project creation.
project(dinfox-dsm)
add_executable(${PROJECT_NAME})

# Use object build mode in all sub-directories.
set(BUILD_MODE "OBJECT")

# Macro to convert options to variables.
macro(add_compilation_flag FLAG_NAME FLAG_DESCRIPTION FLAG_DEFAULT_VALUE)
    option(FLAG_NAME FLAG_DESCRIPTION FLAG_DEFAULT_VALUE)
    set(${FLAG_NAME} ${FLAG_DEFAULT_VALUE} CACHE STRING ${FLAG_DESCRIPTION})
    list(APPEND COMPILATION_FLAGS_LIST ${FLAG_NAME})
endmacro()

# Hardware compilation flags.
set(DSM_BOARD "NONE" CACHE STRING "Board selection")
set(DSM_HW_VERSION "NONE" CACHE STRING "Hardware version")

# Software compilation flags.
add_compilation_flag(DSM_NVM_FACTORY_RESET "Erase NVM registers with new values." OFF)
add_compilation_flag(DSM_NODE_ADDRESS "Node address." 0x7F)
# LVRM.
add_compilation_flag(LVRM_RELAY_CONTROL_FORCED_HARDWARE "To be defined if the relay is controlled by hardware." OFF)
add_compilation_flag(LVRM_MODE_BMS "Enable BMS mode." OFF)
add_compilation_flag(LVRM_BMS_INPUT_VOLTAGE_THL_MV "Low input voltage threshold of the BMS hysteresis." 10000)
add_compilation_flag(LVRM_BMS_INPUT_VOLTAGE_THH_MV "High input voltage threshold of the BMS hysteresis." 12000)
# BPSM.
add_compilation_flag(BPSM_CHARGE_CONTROL_FORCED_HARDWARE "To be defined if the charge is controlled by hardware." OFF)
add_compilation_flag(BPSM_CHARGE_STATUS_FORCED_HARDWARE "To be defined if the charge status is indicated by hardware." ON)
add_compilation_flag(BPSM_BACKUP_CONTROL_FORCED_HARDWARE "To be defined if the backup output is controlled by hardware." ON)
add_compilation_flag(BPSM_CHARGE_SOURCE_VOLTAGE_TH_MV "Minimum source voltage required to enable charging in mV." 6000)
add_compilation_flag(BPSM_CHARGE_TOGGLE_PERIOD_SECONDS "Charge toggle period in seconds." 300)
add_compilation_flag(BPSM_LVF_STORAGE_VOLTAGE_THL_MV "Low storage voltage threshold of the LVF hysteresis." 1000)
add_compilation_flag(BPSM_LVF_STORAGE_VOLTAGE_THH_MV "High storage voltage threshold of the LVF hysteresis." 2000)
add_compilation_flag(BPSM_CVF_STORAGE_VOLTAGE_THL_MV "Low storage voltage threshold of the CVF hysteresis." 1000)
add_compilation_flag(BPSM_CVF_STORAGE_VOLTAGE_THH_MV "High storage voltage threshold of the CVF hysteresis." 2000)
# DDRM.
add_compilation_flag(DDRM_REGULATOR_CONTROL_FORCED_HARDWARE "To be defined if the regulator is controlled by hardware." OFF)
# RRM.
add_compilation_flag(RRM_REGULATOR_CONTROL_FORCED_HARDWARE "To be defined if the regulator is controlled by hardware." OFF)
# SM.
add_compilation_flag(SM_AIN_ENABLE "Enable analog inputs." ON)
add_compilation_flag(SM_DIO_ENABLE "Enable digital IOs." ON)
add_compilation_flag(SM_DIGITAL_SENSORS_ENABLE "Enable digital sensors." ON)
add_compilation_flag(SM_AIN0_GAIN_TYPE "Analog input 0 gain type." ANALOG_GAIN_TYPE_ATTENUATION)
add_compilation_flag(SM_AIN0_GAIN "Analog input 0 gain value." 1)
add_compilation_flag(SM_AIN1_GAIN_TYPE "Analog input 1 gain type." ANALOG_GAIN_TYPE_ATTENUATION)
add_compilation_flag(SM_AIN1_GAIN "Analog input 1 gain value." 1)
add_compilation_flag(SM_AIN2_GAIN_TYPE "Analog input 2 gain type." ANALOG_GAIN_TYPE_ATTENUATION)
add_compilation_flag(SM_AIN2_GAIN "Analog input 2 gain value." 1)
add_compilation_flag(SM_AIN3_GAIN_TYPE "Analog input 3 gain type." ANALOG_GAIN_TYPE_ATTENUATION)
add_compilation_flag(SM_AIN3_GAIN "Analog input 3 gain value." 1)
# GPSM.
add_compilation_flag(GPSM_ACTIVE_ANTENNA "To be defined if the active antenna is used." ON)
add_compilation_flag(GPSM_BACKUP_CONTROL_FORCED_HARDWARE "To be defined if the backup output is controlled by hardware." OFF)
add_compilation_flag(GPSM_TIME_TIMEOUT_SECONDS "Timeout for time acquisition in seconds." 120)
add_compilation_flag(GPSM_GEOLOC_TIMEOUT_SECONDS "Timeout for geolocation acquisition in seconds." 180)
add_compilation_flag(GPSM_TIMEPULSE_FREQUENCY_HZ "Timepulse frequency in Hz." 10000000)
add_compilation_flag(GPSM_TIMEPULSE_DUTY_CYCLE "Timepulse duty cycle in percent." 50)
# MPMCM.
add_compilation_flag(MPMCM_ANALOG_MEASURE_ENABLE "Enable analog measurements." ON)
add_compilation_flag(MPMCM_LINKY_TIC_ENABLE "Enable Linky TIC interface." OFF)
add_compilation_flag(MPMCM_LINKY_TIC_MODE_HISTORIC "Enable Linky TIC historic mode." ON)
add_compilation_flag(MPMCM_LINKY_TIC_MODE_STANDARD "Enable Linky TIC standard mode." OFF)
add_compilation_flag(MPMCM_TRANSFORMER_BLOCK_VC_10_2_6 "Select transformer block VC_10_2_6." OFF)
add_compilation_flag(MPMCM_TRANSFORMER_BLOCK_VB_2_1_6 "Select transformer block VB_2_1_6." ON)
add_compilation_flag(MPMCM_TRANSFORMER_ATTENUATOR_VV "Transformer attenuator value in V/V." 15)
add_compilation_flag(MPMCM_TRANSFORMER_GAIN_DVV "Transformer gain value in 10*V/V." 236)
add_compilation_flag(MPMCM_CURRENT_SENSOR_ATTENUATOR_CH1_VV "Current sensor attenuator channel 1 in V/V." 1)
add_compilation_flag(MPMCM_CURRENT_SENSOR_ATTENUATOR_CH2_VV "Current sensor attenuator channel 2 in V/V." 1)
add_compilation_flag(MPMCM_CURRENT_SENSOR_ATTENUATOR_CH3_VV "Current sensor attenuator channel 3 in V/V." 1)
add_compilation_flag(MPMCM_CURRENT_SENSOR_ATTENUATOR_CH4_VV "Current sensor attenuator channel 4 in V/V." 1)
add_compilation_flag(MPMCM_CURRENT_SENSOR_GAIN_CH1_DAV "Current sensor gain channel 1 in 10*A/V." 50)
add_compilation_flag(MPMCM_CURRENT_SENSOR_GAIN_CH2_DAV "Current sensor gain channel 2 in 10*A/V." 50)
add_compilation_flag(MPMCM_CURRENT_SENSOR_GAIN_CH3_DAV "Current sensor gain channel 3 in 10*A/V." 100)
add_compilation_flag(MPMCM_CURRENT_SENSOR_GAIN_CH4_DAV "Current sensor gain channel 4 in 10*A/V." 200)
# BCM.
add_compilation_flag(BCM_CHARGE_CURRENT_SHUNT_RESISTOR_MOHMS "LTC4013 shunt resistor value in mOhm." 50)
add_compilation_flag(BCM_CHARGE_CONTROL_FORCED_HARDWARE "To be defined if the charge is controlled by hardware" ON)
add_compilation_flag(BCM_CHARGE_STATUS_FORCED_HARDWARE "To be defined if the charge status in indicated by hardware." OFF)
add_compilation_flag(BCM_CHARGE_LED_FORCED_HARDWARE "To be defined if the charge RGB LED is controlled by hardware." ON)
add_compilation_flag(BCM_BACKUP_CONTROL_FORCED_HARDWARE "To be defined if the backup output is controlled by hardware." ON)
add_compilation_flag(BCM_CHARGE_SOURCE_VOLTAGE_TH_MV "Minimum source voltage required to enable charging in mV." 16000)
add_compilation_flag(BCM_CHARGE_TOGGLE_PERIOD_SECONDS "Charge toggle period in seconds." 3600)
add_compilation_flag(BCM_LVF_STORAGE_VOLTAGE_THL_MV "Low storage voltage threshold of the LVF hysteresis." 10000)
add_compilation_flag(BCM_LVF_STORAGE_VOLTAGE_THH_MV "High storage voltage threshold of the LVF hysteresis." 12000)
add_compilation_flag(BCM_CVF_STORAGE_VOLTAGE_THL_MV "Low storage voltage threshold of the CVF hysteresis." 8000)
add_compilation_flag(BCM_CVF_STORAGE_VOLTAGE_THH_MV "High storage voltage threshold of the CVF hysteresis." 10000)

# Hardware specific settings.
if(DSM_BOARD STREQUAL "LVRM")
    # LVRM HW1.0.
    if(DSM_HW_VERSION STREQUAL "HW1_0")   
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=cortex-m0plus -Os")
        set(DSM_MCU "stm32l0xx")
        set(PROJECT_LINKER_SCRIPT "stm32l011x4xx.ld")
    # LVRM HW2.0.
    elseif(DSM_HW_VERSION STREQUAL "HW2_0")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=cortex-m0plus -O1")
        set(DSM_MCU "stm32l0xx")
        set(PROJECT_LINKER_SCRIPT "stm32l031x6xx.ld")
    # Unknown version.
    else()
        message(FATAL_ERROR "Invalid hardware version.")
    endif()
elseif(DSM_BOARD STREQUAL "BPSM")
    # BPSM HW1.0.
    if(DSM_HW_VERSION STREQUAL "HW1_0")   
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=cortex-m0plus -Os")
        set(DSM_MCU "stm32l0xx")
        set(PROJECT_LINKER_SCRIPT "stm32l011x4xx.ld")
    # Unknown version.
    else()
        message(FATAL_ERROR "Invalid hardware version.")
    endif()
elseif(DSM_BOARD STREQUAL "DDRM")
    # DDRM HW1.0.
    if(DSM_HW_VERSION STREQUAL "HW1_0")   
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=cortex-m0plus -Os")
        set(DSM_MCU "stm32l0xx")
        set(PROJECT_LINKER_SCRIPT "stm32l011x4xx.ld")
    # Unknown version.
    else()
        message(FATAL_ERROR "Invalid hardware version.")
    endif()
elseif(DSM_BOARD STREQUAL "UHFM")
    # UHFM HW1.0.
    if(DSM_HW_VERSION STREQUAL "HW1_0")   
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=cortex-m0plus -Os")
        set(DSM_MCU "stm32l0xx")
        set(PROJECT_LINKER_SCRIPT "stm32l041x6xx.ld")
    # UHFM HW2.0.
    elseif(DSM_HW_VERSION STREQUAL "HW2_0")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=cortex-m0plus -O1")
        set(DSM_MCU "stm32l0xx")
        set(PROJECT_LINKER_SCRIPT "stm32l051x8xx.ld")
    # Unknown version.
    else()
        message(FATAL_ERROR "Invalid hardware version.")
    endif()
elseif(DSM_BOARD STREQUAL "RRM")
    # RRM HW1.0.
    if(DSM_HW_VERSION STREQUAL "HW1_0")   
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=cortex-m0plus -Os")
        set(DSM_MCU "stm32l0xx")
        set(PROJECT_LINKER_SCRIPT "stm32l011x4xx.ld")
    # Unknown version.
    else()
        message(FATAL_ERROR "Invalid hardware version.")
    endif()
elseif(DSM_BOARD STREQUAL "SM")
    # SM HW1.0.
    if(DSM_HW_VERSION STREQUAL "HW1_0")   
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=cortex-m0plus -O1")
        set(DSM_MCU "stm32l0xx")
        set(PROJECT_LINKER_SCRIPT "stm32l031x6xx.ld")
    # Unknown version.
    else()
        message(FATAL_ERROR "Invalid hardware version.")
    endif()
elseif(DSM_BOARD STREQUAL "GPSM")
    # GPSM HW1.0.
    if(DSM_HW_VERSION STREQUAL "HW1_0")   
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=cortex-m0plus -O1")
        set(DSM_MCU "stm32l0xx")
        set(PROJECT_LINKER_SCRIPT "stm32l031x6xx.ld")
    # Unknown version.
    else()
        message(FATAL_ERROR "Invalid hardware version.")
    endif()
elseif(DSM_BOARD STREQUAL "MPMCM")
    # MPMCM HW1.0.
    if(DSM_HW_VERSION STREQUAL "HW1_0")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -O0")
        set(DSM_MCU "stm32g4xx")
        set(PROJECT_LINKER_SCRIPT "stm32g441xbxx.ld")
        # Specific CMSIS flags.
        add_compile_definitions(__ARM_FEATURE_DSP=1)
        add_compile_definitions(__PROGRAM_START)
        # Specific CMSIS sources.
        target_sources(${PROJECT_NAME}
            PRIVATE
                drivers/utils/CMSIS-DSP/Source/StatisticsFunctions/arm_mean_f32.c
                drivers/utils/CMSIS-DSP/Source/StatisticsFunctions/arm_rms_f32.c
                drivers/utils/CMSIS-DSP/Source/BasicMathFunctions/arm_mult_f32.c
        )
        # Specific CMSIS includes.
        include_directories(${PROJECT_NAME}
            PRIVATE
                drivers/utils/CMSIS_6/CMSIS/Core/Include
                drivers/utils/CMSIS-DSP/Include
        )
    # Unknown version.
    else()
        message(FATAL_ERROR "Invalid hardware version.")
    endif()
elseif(DSM_BOARD STREQUAL "BCM")
    # BCM HW1.0.
    if(DSM_HW_VERSION STREQUAL "HW1_0")   
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=cortex-m0plus -Os")
        set(DSM_MCU "stm32l0xx")
        set(PROJECT_LINKER_SCRIPT "stm32l011x4xx.ld")
    # Unknown version.
    else()
        message(FATAL_ERROR "Invalid hardware version.")
    endif()
else()
    message(FATAL_ERROR "Invalid board.")
endif()
set(PROJECT_LINKER_PATH "${CMAKE_CURRENT_SOURCE_DIR}/drivers/device/${DSM_MCU}-device/linker")

# Add hardware compilation flags.
add_compile_definitions(
    __DSM_FLAGS_H__
    ${DSM_BOARD}
    ${DSM_HW_VERSION}
)

# Add software compilation flags.
foreach(FLAG ${COMPILATION_FLAGS_LIST})
    # Macros only defined.
    if(${FLAG} STREQUAL ON)
        add_compile_definitions(${FLAG})
    endif()
    # Macros with value.
    if((NOT ${FLAG} STREQUAL OFF) AND (NOT ${FLAG} STREQUAL ON))
        set(${FLAG} "(${${FLAG}})")
        add_compile_definitions(${FLAG}=${${FLAG}}) 
    endif()
endforeach()

# Add project sources files.
target_sources(${PROJECT_NAME}
    PRIVATE
        drivers/peripherals/src/mcu_mapping.c
        drivers/mac/src/lmac_hw.c
        drivers/components/src/led.c
        drivers/components/src/load.c
        drivers/components/src/neom8x_hw.c
        drivers/components/src/s2lp_hw.c
        drivers/components/src/sensors_hw.c
        drivers/components/src/sht3x_hw.c
        drivers/components/src/sx126x_hw.c
        drivers/components/src/tic.c
        drivers/utils/src/terminal_hw.c
        middleware/analog/src/analog.c
        middleware/analog/src/measure.c
        middleware/analog/src/simulation.c
        middleware/cli/src/cli.c
        middleware/digital/src/digital.c
        middleware/gps/src/gps.c
        middleware/node/src/bcm.c
        middleware/node/src/bpsm.c
        middleware/node/src/common.c
        middleware/node/src/ddrm.c
        middleware/node/src/gpsm.c
        middleware/node/src/lvrm.c
        middleware/node/src/mpmcm.c
        middleware/node/src/node.c
        middleware/node/src/node_register.c
        middleware/node/src/rrm.c
        middleware/node/src/sm.c
        middleware/node/src/uhfm.c
        middleware/node/src/una_at_hw.c
        middleware/power/src/power.c
        middleware/sigfox/sigfox-ep-lib/src/core/TI_aes_128_encr_only.c
        middleware/sigfox/src/mcu_api.c
        middleware/sigfox/src/rf_api_s2lp.c
        middleware/sigfox/src/rf_api_sx126x.c
        middleware/sigfox/src/rfe.c
        application/src/main.c
)

# Project include folders.
include_directories(${PROJECT_NAME}
    PRIVATE
        drivers/device/inc
        drivers/registers/inc
        drivers/registers/${DSM_MCU}-registers/inc
        drivers/peripherals/inc
        drivers/peripherals/${DSM_MCU}-drivers/inc
        drivers/utils/inc
        drivers/utils/embedded-utils/inc
        drivers/components/inc
        drivers/components/neom8x-driver/inc
        drivers/components/s2lp-driver/inc
        drivers/components/sht3x-driver/inc
        drivers/components/sx126x-driver/inc
        drivers/mac/inc
        drivers/mac/lmac-driver/inc
        middleware/analog/inc
        middleware/cli/inc
        middleware/digital/inc
        middleware/gps/inc
        middleware/node/inc
        middleware/node/dinfox-registers/inc
        middleware/node/una-lib/inc
        middleware/node/una-at/inc
        middleware/power/inc
        middleware/sigfox/inc
        middleware/sigfox/sigfox-ep-lib/inc
        middleware/sigfox/sigfox-ep-addon-rfp/inc
        application/inc
)

# Add submodules sources files.
add_subdirectory(drivers/device/${DSM_MCU}-device EXCLUDE_FROM_ALL)
add_subdirectory(drivers/peripherals/${DSM_MCU}-drivers EXCLUDE_FROM_ALL)
add_subdirectory(drivers/utils/embedded-utils EXCLUDE_FROM_ALL)
add_subdirectory(drivers/components/neom8x-driver EXCLUDE_FROM_ALL)
add_subdirectory(drivers/components/s2lp-driver EXCLUDE_FROM_ALL)
add_subdirectory(drivers/components/sht3x-driver EXCLUDE_FROM_ALL)
add_subdirectory(drivers/components/sx126x-driver EXCLUDE_FROM_ALL)
add_subdirectory(drivers/mac/lmac-driver EXCLUDE_FROM_ALL)
add_subdirectory(middleware/node/dinfox-registers EXCLUDE_FROM_ALL)
add_subdirectory(middleware/node/una-lib EXCLUDE_FROM_ALL)
add_subdirectory(middleware/node/una-at EXCLUDE_FROM_ALL)
add_subdirectory(middleware/sigfox/sigfox-ep-lib EXCLUDE_FROM_ALL)
add_subdirectory(middleware/sigfox/sigfox-ep-addon-rfp EXCLUDE_FROM_ALL)

# Link libraries.
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${DSM_MCU}-device
        ${DSM_MCU}-drivers
        embedded-utils
        lmac-driver
        neom8x-driver
        s2lp-driver
        sht3x-driver
        sx126x-driver
        dinfox-registers
        una-lib
        una-at
        sigfox_ep_lib_obj
        sigfox_ep_addon_rfp_obj
        c_nano
        nosys
        gcc
)

# Linker and artifact.
include(script/cmake-arm-none-eabi/linker.cmake)
include(script/cmake-arm-none-eabi/artifact.cmake)
