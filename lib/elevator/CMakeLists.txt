file (GLOB SOURCES
	  "*.h"
	  "*.cpp")

source_group("" FILES ${SOURCES})

include_directories (.)

add_library(elevator SHARED ${SOURCES})

set_target_properties(elevator PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

