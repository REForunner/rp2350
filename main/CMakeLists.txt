add_library(main INTERFACE)

file(GLOB_RECURSE SRC_FILES "${CMAKE_CURRENT_LIST_DIR}/*.c *.cpp")
file(GLOB_RECURSE HEAD_FILES "${CMAKE_CURRENT_LIST_DIR}/*.h *.hpp")

target_sources(main INTERFACE ${SRC_FILES} ${HEAD_FILES})

target_include_directories(main INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(main INTERFACE pico_stdlib)
