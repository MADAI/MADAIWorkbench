cmake_minimum_required( VERSION 2.4 )

# Find and Include ParaView
if(NOT ParaView_SOURCE_DIR)
  FIND_PACKAGE(ParaView REQUIRED)
  INCLUDE(${PARAVIEW_USE_FILE})
endif()

GetCMakeCacheValue( "${ParaView_BINARY_DIR}"
  PARAVIEW_USE_VRPN
  VTK_INCLUDE_DIR
  VRPN_INCLUDE_DIR
  VRPN_LIBRARY
)

# Get the path to the VRPN server library
get_filename_component( VRPN_PATH "${VRPN_LIBRARY}" PATH )
set( VRPN_SERVER_LIBRARY "${VRPN_PATH}/libvrpnserver.a" )

cmake_policy(SET CMP0005 NEW)

if (PARAVIEW_USE_VRPN)

    # Look to see if we are building against VRPN that exists in a CMake-
    # configured build tree.
    set( VRPN_CMAKECACHE_FILE "${VRPN_PATH}/CMakeCache.txt" )

    if ( UNIX AND EXISTS "${VRPN_CMAKECACHE_FILE}" )
      # We need to add a compiler definition to tell VRPN that we want to use
      # the vrpn_Configure.h file in the CMake build directory instead of the
      # source directory. We need only do this on Mac and Linux
      add_definitions(-DVRPN_USING_CMAKE="${VRPN_PATH}/vrpn_Configure.h")
    endif()

    include_directories(${VRPN_INCLUDE_DIR})
    set(VRPN_SOURCE_FILES
      vtkVRPNClient.cxx
      vtkVRPNServer.cxx
      )

    if(NOT EXISTS VRPN_INCLUDE_DIR)
      find_path(VRPN_INCLUDE_DIR vrpn_Tracker.h)
    endif()
    find_library(VRPN_LIBRARY vrpn)

endif(PARAVIEW_USE_VRPN)

qt4_wrap_cpp( MOC_SRCS
  pqVRPNClientServerStarter.h
  vtkVRPNClient.h
  vtkVRPNServer.h
)

include( ${QT_USE_FILE} )

include_directories(
  ${VTK_INCLUDE_DIR}
  ${PARAVIEW_INCLUDE_DIRS}
  ${PARAVIEW_GUI_INCLUDE_DIRS}
  ${PARAVIEW_KWSYS_INCLUDE_DIRS}
  ${pqCore_SOURCE_DIR}
  ${pqCore_BINARY_DIR}
  ${pqComponents_SOURCE_DIR}
  ${pqComponents_BINARY_DIR}
  )

if (PARAVIEW_USE_VRPN)

  if ( NOT APPLE AND UNIX )
    find_package( USB1 REQUIRED )
  endif()

  add_paraview_auto_start( IFACES IFACE_SRCS CLASS_NAME pqVRPNClientServerStarter
    STARTUP onStartup
    SHUTDOWN onShutdown
    )

  add_paraview_plugin(
    VRPNDevices "1.0"
    GUI_INTERFACES ${IFACES}
    SOURCES
      pqVRPNClientServerStarter.cxx
      vtkVRPNClient.cxx
      vtkVRPNServer.cxx
      ${MOC_SRCS}
      ${IFACE_SRCS}
    )
  target_link_libraries( VRPNDevices
    "${VRPN_LIBRARY}"
    "${VRPN_SERVER_LIBRARY}"
    "${USB1_LIBRARY}"
    )

endif()
