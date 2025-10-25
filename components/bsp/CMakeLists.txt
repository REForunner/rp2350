add_library(bsp INTERFACE)

file(GLOB_RECURSE SRC_FILES "${CMAKE_CURRENT_LIST_DIR}/*.c *.cpp")
file(GLOB_RECURSE HEAD_FILES "${CMAKE_CURRENT_LIST_DIR}/*.h *.hpp")

target_sources(bsp INTERFACE ${SRC_FILES} ${HEAD_FILES})

target_include_directories(bsp INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(bsp INTERFACE pico_stdlib hardware_dma)
