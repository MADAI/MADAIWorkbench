project( MADAIWorkbench )
cmake_minimum_required(VERSION 2.8)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin/ CACHE PATH
  "Single output directory for building all plugins."
  )

find_package(ParaView REQUIRED)
include(${PARAVIEW_USE_FILE})

add_subdirectory(Plugins)
