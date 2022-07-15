file(GLOB FILES *.c *.h)

# rest of your project

add_library(ssd1306-display	${FILES})

target_link_libraries(ssd1306-display pico_stdlib hardware_i2c)

target_include_directories(ssd1306-display PUBLIC ../include/)