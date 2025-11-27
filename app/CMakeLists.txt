add_library(app INTERFACE)

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
	target_sources(app INTERFACE ${SRC_FILES})
endif()

if(HEAD_FILES)
	# Adding headers to target_sources is optional, but can help IDEs.
	target_sources(app INTERFACE ${HEAD_FILES})
endif()

target_compile_definitions(app INTERFACE USE_PIO_SWD=${USE_PIO_SWD} PICO_LINK_SWDIO=${SWDIO_PIN} PICO_LINK_SWCLK=${SWCLK_PIN})

target_include_directories(app INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(app INTERFACE pico_stdlib freertos hardware_pio hardware_dma hardware_clocks main)
