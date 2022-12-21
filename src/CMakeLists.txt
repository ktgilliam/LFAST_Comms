########## LFAST Controller Utilities ##############

# set our include directories to look for header files
include_directories( ${CMAKE_BINARY_DIR} )
include_directories( ${CMAKE_CURRENT_BINARY_DIR} )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

#=================================================================================================#
#========================================= project libraries =====================================#
#=================================================================================================#
add_library(PID_Controller SHARED PID_Controller.cc)
# add_library(astro_math SHARED astro_math.cc)