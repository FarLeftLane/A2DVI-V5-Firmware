# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
# We have a custom board header for PV
set(PICO_BOARD A2DVI_V5)
# Add the directory containing the custom board header to the search path
set(PICO_BOARD_HEADER_DIRS "${CMAKE_CURRENT_LIST_DIR}/firmware/boards")


# Pull in Raspberry Pi Pico SDK (must be before project)

include(pico_sdk_import.cmake)
# firmware version
set(FW_VERSION "5.0.1a")
message(STATUS "Building firmware version ${FW_VERSION}")
project(A2DVI-V5-v5.0.1a C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# enable compiler warnings
add_compile_options(-Wall)

# At 640pixels each TMDS buffer requires 3840bytes
add_compile_options(-DDVI_N_TMDS_BUFFERS=8)

add_compile_options(-DFW_VERSION="${FW_VERSION}")

# Enable the debug counters FEATURE_DEBUG_COUNTER
add_compile_options(-DFEATURE_DEBUG_COUNTER=1)

# Add executable. Default name is the project name, version 0.1

add_executable(${PROJECT_NAME} 
    firmware/main.c
    firmware/applebus/abus.c
    firmware/applebus/abus_setup.c
    firmware/applebus/buffers.c

    firmware/dvi/a2dvi.c
    firmware/dvi/tmds.c
    firmware/dvi/tmds_lores.c
    firmware/dvi/tmds_hires.c
    firmware/dvi/tmds_dhgr.c

    firmware/render/render.c
    firmware/render/render_splash.c
    firmware/render/render_debug.c
    firmware/render/render_text.c
    firmware/render/render_lores.c
    firmware/render/render_shr.c
    firmware/render/render_dgr.c
    firmware/render/render_hires.c
    firmware/render/render_dhgr.c
    firmware/render/render_videx.c

    firmware/config/config.c
    firmware/config/device_regs.c

    firmware/videx/videx_vterm.c

    firmware/menu/menu.c

    firmware/debug/debug.c
    firmware/util/dmacopy.c

    firmware/fonts/textfont.c
    firmware/fonts/iie_us_enhanced.c
    firmware/fonts/iie_us_unenhanced.c
    firmware/fonts/iie_us_reactive.c
    firmware/fonts/iie_uk_enhanced.c
    firmware/fonts/iie_fr_ca_enhanced.c
    firmware/fonts/iie_de_enhanced.c
    firmware/fonts/iie_spanish_enhanced.c
    firmware/fonts/iie_it_enhanced.c
    firmware/fonts/iie_hebrew_enhanced.c
    firmware/fonts/iie_se_fi_enhanced.c
    firmware/fonts/clone_pravetz_cyrillic.c
    firmware/fonts/iiplus_us.c
    firmware/fonts/iiplus_videx_lowercase1.c
    firmware/fonts/iiplus_videx_lowercase2.c
    firmware/fonts/iiplus_pigfont.c
    firmware/fonts/iiplus_jp_katakana.c

    firmware/fonts/videx/videx_normal.c
    firmware/fonts/videx/videx_inverse.c
    firmware/fonts/videx/videx_apl.c
    firmware/fonts/videx/videx_french.c
    firmware/fonts/videx/videx_super_sub.c
    firmware/fonts/videx/videx_uppercase.c
    firmware/fonts/videx/videx_epson.c
    firmware/fonts/videx/videx_german.c
    firmware/fonts/videx/videx_katakana.c
    firmware/fonts/videx/videx_spanish.c
    firmware/fonts/videx/videx_symbol.c

    libraries/libdvi/dvi.c
    libraries/libdvi/dvi_serialiser.c
    libraries/libdvi/dvi_timing.c
    libraries/libdvi/tmds_encode.S
    libraries/libdvi/tmds_encode.c
 )

pico_set_program_name(${PROJECT_NAME} "A2DVI")
pico_set_program_version(${PROJECT_NAME} "5.0a")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(${PROJECT_NAME} 0)
pico_enable_stdio_usb(${PROJECT_NAME} 0)

# Add the standard library to the build
target_link_libraries(${PROJECT_NAME}
    pico_base_headers
    pico_runtime_init
    pico_runtime
    pico_multicore
    pico_util

    hardware_flash
    hardware_vreg
    hardware_clocks
    hardware_dma
    hardware_pio
    hardware_interp
    hardware_pwm

    pico_stdlib
)

# Add the standard include files to the build
target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/firmware
)

target_include_directories(${PROJECT_NAME} PRIVATE libraries/libdvi)

# PIO code
pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/firmware/applebus/abus.pio)
pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/libraries/libdvi/dvi_serialiser.pio)
pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/libraries/libdvi/tmds_encode_1bpp.pio)

pico_add_extra_outputs(${PROJECT_NAME})

# use platform-specific linker script and run from RAM
pico_set_binary_type(${PROJECT_NAME} copy_to_ram)
pico_set_linker_script(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/firmware/scripts/copy_to_ram_custom_rp2350.ld)
