add_library(log INTERFACE)

target_sources(
    log
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}/log.h
              ${CMAKE_CURRENT_LIST_DIR}/log.c
              )

target_include_directories(log INTERFACE ${CMAKE_CURRENT_LIST_DIR})
