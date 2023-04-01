# Read all source files from the 'src' directory
FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.*)

# Register all source files
idf_component_register(SRCS ${app_sources})