project( MADAIWorkbench )
cmake_minimum_required(VERSION 2.8.5)

option(BUILD_APPLICATION "Build the MADAI Workbench application." OFF)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin CACHE PATH
  "Single output directory for building all executables."
  )
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin CACHE PATH
  "Single output directory for building all plugins."
  )

find_package(ParaView REQUIRED)
include(${PARAVIEW_USE_FILE})

add_subdirectory(Plugins)

if(BUILD_APPLICATION)
  add_subdirectory(Application)
endif()