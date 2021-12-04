cmake_minimum_required(VERSION 3.19)

# FIXME: CMake keeps trying to build a test program for Mac using arm-none-eabi-gcc
set(CMAKE_C_COMPILER_WORKS 1)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
SET(CMAKE_GENERATOR "Unix Makefiles")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CROSSCOMPILING 1)

set(CMAKE_C_COMPILER arm-none-eabi-gcc CACHE PATH "" FORCE)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc CACHE PATH "" FORCE)

project(main C ASM)

set(UCOS_SOURCES
    uC-OS3/Source/os_cfg_app.c
    uC-OS3/Source/os_core.c
    uC-OS3/Source/os_dbg.c
    uC-OS3/Source/os_flag.c
    uC-OS3/Source/os_mem.c
    uC-OS3/Source/os_msg.c
    uC-OS3/Source/os_mutex.c
    uC-OS3/Source/os_prio.c
    uC-OS3/Source/os_q.c
    uC-OS3/Source/os_sem.c
    uC-OS3/Source/os_stat.c
    uC-OS3/Source/os_task.c
    uC-OS3/Source/os_tick.c
    uC-OS3/Source/os_time.c
    uC-OS3/Source/os_tmr.c
    uC-OS3/Source/os_var.c
    uC-OS3/Ports/ARM-Cortex-M/ARMv7-M/os_cpu_c.c
    uC-OS3/Ports/ARM-Cortex-M/ARMv7-M/GNU/os_cpu_a.s
)

set(UCCPU_SOURCES
    uC-CPU/cpu_core.c
    uC-CPU/ARM-Cortex-M/ARMv7-M/cpu_c.c
    uC-CPU/ARM-Cortex-M/ARMv7-M/GNU/cpu_a.s
)

set(UCLIB_SOURCES
    uC-Lib/lib_ascii.c
    uC-Lib/lib_math.c
    uC-Lib/lib_mem.c
    uC-Lib/lib_str.c
)

set(BSP_SOURCES
    BSP/ST/STM32F7xx_Nucleo_144/bsp.c
    BSP/ST/STM32F7xx_Nucleo_144/bsp_led.c
    BSP/ST/STM32F7xx_Nucleo_144/bsp_sensor.c
    BSP/ST/STM32F7xx_Nucleo_144/bsp_uart.c
    BSP/ST/STM32F7xx_Nucleo_144/WeatherShield/i2c.c
    BSP/ST/STM32F7xx_Nucleo_144/WeatherShield/MS8607_Generic_C_Driver/ms8607.c
    # NOTE: Files in "Templates" are normally copied into project for customization, but not necessary for this project
    BSP/ST/STM32F7xx_Nucleo_144/STM32CubeF7/Drivers/CMSIS/Device/ST/STM32F7xx/Source/Templates/gcc/startup_stm32f767xx.s
    BSP/ST/STM32F7xx_Nucleo_144/STM32CubeF7/Drivers/CMSIS/Device/ST/STM32F7xx/Source/Templates/system_stm32f7xx.c
    BSP/ST/STM32F7xx_Nucleo_144/STM32CubeF7/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_cortex.c
    BSP/ST/STM32F7xx_Nucleo_144/STM32CubeF7/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_dma.c
    BSP/ST/STM32F7xx_Nucleo_144/STM32CubeF7/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_gpio.c
    BSP/ST/STM32F7xx_Nucleo_144/STM32CubeF7/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_i2c.c
    BSP/ST/STM32F7xx_Nucleo_144/STM32CubeF7/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_pwr.c
    BSP/ST/STM32F7xx_Nucleo_144/STM32CubeF7/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_pwr_ex.c
    BSP/ST/STM32F7xx_Nucleo_144/STM32CubeF7/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_rcc.c
    BSP/ST/STM32F7xx_Nucleo_144/STM32CubeF7/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_rcc_ex.c
    BSP/ST/STM32F7xx_Nucleo_144/STM32CubeF7/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_uart.c
)

set(SOURCES
    Source/main.c
    Source/app_task/app_task.c
    Source/logger_task/logger_task.c
    Source/os_app_hooks/os_app_hooks.c
    Source/sensor_task/sensor_task.c
    ${BSP_SOURCES}
    ${UCOS_SOURCES}
    ${UCCPU_SOURCES}
    ${UCLIB_SOURCES}
)

include_directories(
    Cfg
    Source/app_task
    Source/logger_task
    Source/os_app_hooks
    Source/sensor_task
    BSP/ST/STM32F7xx_Nucleo_144/
    BSP/ST/STM32F7xx_Nucleo_144/Cfg
    BSP/ST/STM32F7xx_Nucleo_144/STM32CubeF7/Drivers/CMSIS/Core/Include
    BSP/ST/STM32F7xx_Nucleo_144/STM32CubeF7/Drivers/CMSIS/Device/ST/STM32F7xx/Include
    BSP/ST/STM32F7xx_Nucleo_144/STM32CubeF7/Drivers/STM32F7xx_HAL_Driver/Inc
    BSP/ST/STM32F7xx_Nucleo_144/WeatherShield
    BSP/ST/STM32F7xx_Nucleo_144/WeatherShield/MS8607_Generic_C_Driver
    uC-OS3/Source
    uC-OS3/Ports/ARM-Cortex-M/ARMv7-M/GNU
    uC-CPU
    uC-CPU/ARM-Cortex-M/ARMv7-M/GNU
    uC-Lib
)

# For some reason this file needs to be relative to the build directory
set(LDSCRIPT ../BSP/ST/STM32F7xx_Nucleo_144/GNU/stm32f767zitx.ld)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -Wall")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-parameter -Wno-misleading-indentation -Wno-enum-compare")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=cortex-m7 -mthumb -mlittle-endian -mthumb-interwork")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mfloat-abi=hard -mfpu=fpv4-sp-d16")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DSTM32F767xx -DUSE_HAL_DRIVER")
set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections -Wl,-T${LDSCRIPT}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --specs=nano.specs --specs=nosys.specs")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -u _printf_float")

add_executable(main.elf ${SOURCES})

# Link math library, needed by the MS8607 driver
target_link_libraries(main.elf PRIVATE m)
