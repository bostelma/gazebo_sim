cmake_minimum_required(VERSION 3.10.2 FATAL_ERROR)

#============================================================================
# Initialize the project
#============================================================================
project(Drone)

#============================================================================
# Find gz-cmake
#============================================================================
find_package(gz-cmake3 REQUIRED)

#============================================================================
# Configure the project
#============================================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

gz_configure_project(VERSION_SUFFIX)

#============================================================================
# Search for project-specific dependencies
#============================================================================

#--------------------------------------
# Find gz-plugin
gz_find_package(gz-plugin2 REQUIRED COMPONENTS register)
set(GZ_PLUGIN_VER ${gz-plugin2_VERSION_MAJOR})

#--------------------------------------
# Find gz-sim7
gz_find_package(gz-sim7 REQUIRED)
set(GZ_SIM_VER ${gz-sim7_VERSION_MAJOR})

# Include dirs
include_directories(include)

#============================================================================
# Create the library
#============================================================================

add_library(${PROJECT_NAME} SHARED
    src/drone.cpp)

target_link_libraries(${PROJECT_NAME}
  PUBLIC gz-plugin${GZ_PLUGIN_VER}::gz-plugin${GZ_PLUGIN_VER}
  PUBLIC gz-sim${GZ_SIM_VER}::gz-sim${GZ_SIM_VER}
  ${PROTO_SRC})
