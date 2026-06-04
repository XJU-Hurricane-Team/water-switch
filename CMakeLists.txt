set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
cmake_minimum_required(VERSION 3.20)

set(PREFIX "arm-none-eabi-")

if (WIN32)
    MESSAGE(STATUS "Now is windows!")
    set(CMAKE_C_COMPILER ${PREFIX}gcc.exe)
    set(CMAKE_CXX_COMPILER ${PREFIX}g++.exe)
    set(CMAKE_ASM_COMPILER ${PREFIX}gcc.exe)
    set(CMAKE_AR ${PREFIX}ar.exe)
    set(CMAKE_OBJCOPY ${PREFIX}objcopy.exe)
    set(CMAKE_OBJDUMP ${PREFIX}objdump.exe)
    set(SIZE ${PREFIX}size.exe)
elseif (UNIX)
    MESSAGE(STATUS "Now is UNIX-like OS!")
    set(CMAKE_C_COMPILER ${PREFIX}gcc)
    set(CMAKE_CXX_COMPILER ${PREFIX}g++)
    set(CMAKE_ASM_COMPILER ${PREFIX}gcc)
    set(CMAKE_AR ${PREFIX}ar)
    set(CMAKE_OBJCOPY ${PREFIX}objcopy)
    set(CMAKE_OBJDUMP ${PREFIX}objdump)
    set(SIZE ${PREFIX}size)
else ()
    MESSAGE(STATUS "Unsupported system!")
endif ()

project(WaterSwitch C CXX ASM)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 23)

add_compile_options(
        -mcpu=cortex-m3
        -mthumb
)

add_compile_options(
        -ffunction-sections
        -fdata-sections
        -funsigned-char
        -fno-common
        -ffast-math
        --specs=nano.specs
        --specs=nosys.specs
        -MMD
)

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    message(STATUS "Maximum optimization for speed")
    add_compile_options(-Ofast -flto)
    add_link_options(-flto)
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    message(STATUS "Debug mode")
    add_compile_options(-Og -g)
else ()
    message(STATUS "Minimal optimization, debug info included")
    add_compile_options(-Og -g)
endif ()

add_compile_options(-Wall)

include_directories(
        STM32CubeMX/Core/Inc

        STM32CubeMX/Drivers/CMSIS/Include
        STM32CubeMX/Drivers/CMSIS/Device/ST/STM32F1xx/Include
        STM32CubeMX/Drivers/STM32F1xx_HAL_Driver/Inc

        Bsp/
        App/Inc
)

add_definitions(
        -DSTM32F103xB
        -DUSE_HAL_DRIVER
        -DUSE_FULL_LL_DRIVER
)

file(GLOB_RECURSE SOURCES
        "STM32CubeMX/Core/Src/*.c"
        "STM32CubeMX/Core/Startup/startup_stm32f103c8tx.s"

        "STM32CubeMX/Drivers/Bsp/*.c"
        "STM32CubeMX/Drivers/STM32F1xx_HAL_Driver/Src/*.c"

        "Bsp/*.c"
        "App/Src/*.c"
)


## FreeRTOS Configuration #####################################################
add_library(freertos_config INTERFACE)
target_include_directories(freertos_config
        INTERFACE
        "App/Inc"
)
set(FREERTOS_KERNEL_PATH "FreeRTOS")
set(FREERTOS_HEAP "4" CACHE STRING "" FORCE)
set(FREERTOS_PORT "GCC_ARM_CM3" CACHE STRING "" FORCE)
add_subdirectory(${FREERTOS_KERNEL_PATH} FreeRTOS-Kernel)
## End of FreeRTOS ############################################################

set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/STM32CubeMX/STM32F103CBTX_FLASH.ld)

add_link_options(
        -mcpu=cortex-m3
        -mfloat-abi=soft
        -mthumb
        -Wl,--gc-sections
        -Wl,--print-memory-usage
        --specs=nano.specs
        --specs=nosys.specs
        -Wl,-Map=${PROJECT_NAME}.map
)

add_link_options(-T ${LINKER_SCRIPT})

add_executable(${PROJECT_NAME}.elf ${SOURCES} ${LINKER_SCRIPT})

target_link_libraries(${PROJECT_NAME}.elf PRIVATE
        freertos_kernel
)

set(HEX_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.hex)
set(BIN_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.bin)
set(LST_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.lst)

add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:${PROJECT_NAME}.elf> ${HEX_FILE}
        COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${PROJECT_NAME}.elf> ${BIN_FILE}
        COMMAND ${CMAKE_OBJDUMP} --all-headers --demangle --disassemble $<TARGET_FILE:${PROJECT_NAME}.elf> > ${LST_FILE}
        COMMENT "Building ${HEX_FILE}\nBuilding ${BIN_FILE}"
)