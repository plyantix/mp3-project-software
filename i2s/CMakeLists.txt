add_library(i2s STATIC i2s.c)

target_include_directories(i2s PUBLIC ${CMAKE_CURRENT_LIST_DIR})

# Generate the PIO header for the library (path relative to this CMake file)
pico_generate_pio_header(i2s ${CMAKE_CURRENT_LIST_DIR}/audio_i2s.pio)

target_link_libraries(i2s PUBLIC
        pico_stdlib
        hardware_dma
        hardware_irq
        hardware_pio
        hardware_clocks
)
