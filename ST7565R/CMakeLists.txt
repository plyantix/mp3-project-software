add_library(ST7565R STATIC ST7565R.c)

target_include_directories(ST7565R PUBLIC ${CMAKE_CURRENT_LIST_DIR})


target_link_libraries(ST7565R PUBLIC
        pico_stdlib
        hardware_spi
        hardware_gpio
)
