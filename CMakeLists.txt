project( MADAIWorkbench )
cmake_minimum_required(VERSION 2.8.4)

include( "CMake/GetCMakeCacheValue.cmake" )

option(BUILD_APPLICATION "Build the MADAI Workbench application." OFF)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin CACHE PATH
  "Single output directory for building all executables."
  )
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin CACHE PATH
  "Single output directory for building all plugins."
  )
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin CACHE PATH
  "Single output directory for building all plugins on Windows."
  )

find_package(ParaView REQUIRED)
include(${PARAVIEW_USE_FILE})

# Delete the list of ParaView plugins to prevent the same plugins from
# being listed over and over each time the project is reconfigured
set(PARAVIEW_AUTOLOAD_PLUGINLIST "" CACHE INTERNAL "Docs" FORCE)
set(PARAVIEW_PLUGINLIST "" CACHE INTERNAL "Docs" FORCE)

get_filename_component(ParaView_SOURCE_DIR "${ParaView_CMAKE_DIR}/.." ABSOLUTE)
get_filename_component(ParaView_BINARY_DIR "${ParaView_DIR}" ABSOLUTE)

if(BUILD_APPLICATION)
  add_subdirectory(Application)
endif()
