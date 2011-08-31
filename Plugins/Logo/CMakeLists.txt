# Create this plugin as an auto-start plugin. Auto start plugins
# provide callbacks that get called when the plugin is loaded and when
# the application shuts down.

include_directories(
  ${VTK_INCLUDE_DIR}
  ${PARAVIEW_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${ParaView_SOURCE_DIR}/Qt/Core
  ${ParaView_SOURCE_DIR}/Qt/Components
  ${ParaView_BINARY_DIR}/Qt/Core
  ${ParaView_BINARY_DIR}/Qt/Components
  )

if (NOT ParaView_BINARY_DIR)
  find_package(ParaView REQUIRED)
  include(${PARAVIEW_USE_FILE})
endif(NOT ParaView_BINARY_DIR)

qt4_wrap_cpp(MOC_SRCS
  pqLogoStarter.h
  )

include(${QT_USE_FILE})

add_paraview_auto_start(IFACES IFACE_SRCS CLASS_NAME pqLogoStarter
  STARTUP onStartup
  SHUTDOWN onShutdown
  )

# Create a plugin for this starter.

add_paraview_plugin(
  LogoPlugin "1.0"
  AUTOLOAD ON
  GUI_INTERFACES ${IFACES}
  SOURCES pqLogoStarter.cxx
    ${MOC_SRCS}
    ${VRPN_SOURCE_FILES}
    ${RCS_SRCS} ${IFACE_SRCS}
  SERVER_MANAGER_XML LogoTextSource.xml
  )

target_link_libraries(LogoPlugin pqApplicationComponents)