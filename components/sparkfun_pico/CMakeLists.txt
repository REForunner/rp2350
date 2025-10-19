add_library(sparkfun_pico INTERFACE)

target_sources(
    sparkfun_pico
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}/sfe_psram.h
              ${CMAKE_CURRENT_LIST_DIR}/sfe_psram.c
              )

target_include_directories(sparkfun_pico INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(sparkfun_pico INTERFACE pico_stdlib hardware_spi  pico_flash hardware_exception hardware_sync)
