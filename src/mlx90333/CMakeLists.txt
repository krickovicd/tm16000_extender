file(GLOB FILES *.c *.h)

# rest of your project

add_library(mlx_90333_sensor	${FILES})

target_link_libraries(mlx_90333_sensor pico_stdlib hardware_spi)

target_include_directories(mlx_90333_sensor PUBLIC ../include/)