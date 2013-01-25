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

if (PARAVIEW_USE_VRPN)
    include_directories(${VRPN_INCLUDE_DIR})
    set(VRPN_SOURCE_FILES
      vtkVRPNServer.cxx
      )

    if(NOT EXISTS VRPN_INCLUDE_DIR)
      find_path(VRPN_INCLUDE_DIR vrpn_Tracker.h)
    endif()
    find_library(VRPN_LIBRARY vrpn)

endif(PARAVIEW_USE_VRPN)

qt4_wrap_cpp( MOC_SRCS
  pqVRPNServerStarter.h
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

  add_paraview_auto_start( IFACES IFACE_SRCS CLASS_NAME pqVRPNServerStarter
    STARTUP onStartup
    SHUTDOWN onShutdown
    )

  add_paraview_plugin(
    VRPNServer "1.0"
    GUI_INTERFACES ${IFACES}
    SOURCES
      pqVRPNServerStarter.cxx
      vtkVRPNServer.cxx
      ${MOC_SRCS}
      ${IFACE_SRCS}
    )
  target_link_libraries( VRPNServer
    "${VRPN_LIBRARY}"
    "${VRPN_SERVER_LIBRARY}"
    "${USB1_LIBRARY}"
    )

endif()
