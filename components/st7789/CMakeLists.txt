add_library(st7789 INTERFACE)

# Collect C/C++ source files in this directory (recursive).
# Note: provide each glob pattern as a separate argument; combining
# patterns into a single string is incorrect and doesn't match files.
file(GLOB_RECURSE SRC_FILES
	"${CMAKE_CURRENT_LIST_DIR}/*.c"
	"${CMAKE_CURRENT_LIST_DIR}/*.cpp"
)
file(GLOB_RECURSE HEAD_FILES
	"${CMAKE_CURRENT_LIST_DIR}/*.h"
	"${CMAKE_CURRENT_LIST_DIR}/*.hpp"
)

# Add sources/headers to the INTERFACE target only when files exist.
if(SRC_FILES)
	target_sources(st7789 INTERFACE ${SRC_FILES})
endif()

if(HEAD_FILES)
	# Adding headers to target_sources is optional, but can help IDEs.
	target_sources(st7789 INTERFACE ${HEAD_FILES})
endif()

target_include_directories(st7789 INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(st7789 INTERFACE pico_stdlib)

# Optional configuration overrides
if(DEFINED USE_DRIVER_ST7789_FONT)
    target_compile_definitions(st7789 INTERFACE USE_DRIVER_ST7789_FONT=${USE_DRIVER_ST7789_FONT})
endif()

if(DEFINED USE_ST7789_RESET_PIN)
    target_compile_definitions(st7789 INTERFACE USE_ST7789_RESET_PIN=${USE_ST7789_RESET_PIN})
endif()

if(DEFINED ST7789_BUFFER_SIZE)
    target_compile_definitions(st7789 INTERFACE ST7789_BUFFER_SIZE=${ST7789_BUFFER_SIZE})
endif()