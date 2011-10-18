project( MADAIWorkbench )
cmake_minimum_required(VERSION 2.8.4)

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

get_filename_component(ParaView_SOURCE_DIR "${ParaView_CMAKE_DIR}/.." ABSOLUTE)

add_subdirectory(Plugins)

if(BUILD_APPLICATION)
  add_subdirectory(Application)
endif()
