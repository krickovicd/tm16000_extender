set(FAMILY rp2040)
set(BOARD pico_sdk)
set(TINYUSB_FAMILY_PROJECT_NAME_PREFIX "tinyusb_dev_")
#add_subdirectory(${PICO_TINYUSB_PATH}/examples/device tinyusb_device_examples)


cmake_minimum_required(VERSION 3.13)

add_subdirectory(display)
add_subdirectory(mlx90333)

add_executable(tm16000_extender)

target_sources(tm16000_extender PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/main.c
        ${CMAKE_CURRENT_LIST_DIR}/usb_descriptors.c
        )

# Make sure TinyUSB can find tusb_config.h
target_include_directories(tm16000_extender PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/display
        ${CMAKE_CURRENT_LIST_DIR}/mlx90333
        )

# In addition to pico_stdlib required for common PicoSDK functionality, add dependency on tinyusb_device
# for TinyUSB device support and tinyusb_board for the additional board support library used by the example
target_link_libraries(tm16000_extender PUBLIC pico_stdlib tinyusb_device tinyusb_board ssd1306-display hardware_i2c hardware_spi mlx_90333_sensor)

# Uncomment this line to enable fix for Errata RP2040-E5 (the fix requires use of GPIO 15)
#target_compile_definitions(tm16000_extender PUBLIC PICO_RP2040_USB_DEVICE_ENUMERATION_FIX=1)

pico_add_extra_outputs(tm16000_extender)

# add url via pico_set_program_url
example_auto_set_url(tm16000_extender)
