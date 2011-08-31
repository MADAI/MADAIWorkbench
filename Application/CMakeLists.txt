# Set the version number
set(MW_VERSION_MAJOR 1)
set(MW_VERSION_MINOR 0)
set(MW_VERSION_PATCH 0)
set(MW_VERSION_STRING "${MW_VERSION_MAJOR}.${MW_VERSION_MINOR}.${MW_VERSION_PATCH}")

# Now build the MADAI workbench as a branded ParaView
include(${ParaView_CMAKE_DIR}/ParaViewBranding.cmake)
include(${ParaView_CMAKE_DIR}/ParaViewPlugins.cmake)

set(PARAVIEW_APPLICATION_DIR "${ParaView_SOURCE_DIR}/Applications/ParaView")

QT4_WRAP_CPP(MOC_BUILT_SOURCES
  ${PARAVIEW_APPLICATION_DIR}/ParaViewMainWindow.h)

QT4_WRAP_UI(UI_BUILT_SOURCES
  ${PARAVIEW_APPLICATION_DIR}/ParaViewMainWindow.ui
)

source_group("Generated" FILES
  ${MOC_BUILT_SOURCES}
  ${UI_BUILT_SOURCES}
)

set (QT_USE_QTNETWORK TRUE)
include(${QT_USE_FILE})

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR})

set(ParaView_SOURCE_FILES
  ${PARAVIEW_APPLICATION_DIR}/ParaViewMainWindow.cxx
  ${PARAVIEW_APPLICATION_DIR}/ParaViewMainWindow.h
  ${MOC_BUILT_SOURCES}
  ${UI_BUILT_SOURCES})

#------------------------------------------------------------------------------
# Add install rules for required system runtimes such as MSVCRxx.dll
set (CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP ON)
include(InstallRequiredSystemLibraries)
if (CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS)
  install(FILES ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
    DESTINATION ${PV_INSTALL_BIN_DIR}
    PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ
    COMPONENT Runtime)

  # Install the runtimes to the lib dir as well since python modules are
  # installed in that directory and the manifest files need to present there as
  # well.
  install(FILES ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
    DESTINATION ${PV_INSTALL_LIB_DIR}
    PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ
    COMPONENT Runtime)
endif (CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS)

# List the plugins that will be installed and available in the workbench
set(MW_PLUGINS
  LogoPlugin
  BinningFilter
  GaussianScalarSplatter
  BooleanOperation
)

#------------------------------------------------------------------------------
set(APP_NAME "MADAIWorkbench")
build_paraview_client("${APP_NAME}"
  APPLICATION_NAME "MADAIWorkbench"
  TITLE "MADAI Workbench ${MW_VERSION_STRING} ${PARAVIEW_BUILD_ARCHITECTURE}-bit"
  ORGANIZATION  "Modeling and Data Analysis Initiative"
  VERSION_MAJOR ${MW_VERSION_MAJOR}
  VERSION_MINOR ${MW_VERSION_MINOR}
  VERSION_PATCH ${MW_VERSION_PATCH}
  SPLASH_IMAGE "${PROJECT_SOURCE_DIR}/Application/MADAILogo.png"
  PVMAIN_WINDOW ParaViewMainWindow
  PVMAIN_WINDOW_INCLUDE ParaViewMainWindow.h
  BUNDLE_ICON   "${PROJECT_SOURCE_DIR}/Application/MacIcon.icns"
  APPLICATION_ICON  "${PROJECT_SOURCE_DIR}/Application/WinIcon.ico"
  GUI_CONFIGURATION_XMLS
      ${PARAVIEW_APPLICATION_DIR}/ParaViewSources.xml
      ${PARAVIEW_APPLICATION_DIR}/ParaViewFilters.xml
      ${PARAVIEW_APPLICATION_DIR}/ParaViewReaders.xml
      ${PARAVIEW_APPLICATION_DIR}/ParaViewWriters.xml
      ${PARAVIEW_EXTRA_GUI_RESOURCES} # Add any compiled in GUI configuration xmls
  COMPRESSED_HELP_FILE "${ParaView_BINARY_DIR}/Documentation/paraview.qch"
  SOURCES ${ParaView_SOURCE_FILES}
  REQUIRED_PLUGINS ${MW_PLUGINS}
  INSTALL_BIN_DIR "${PV_INSTALL_BIN_DIR}"
  INSTALL_LIB_DIR "${PV_INSTALL_LIB_DIR}"
)

# link enabled plugins if not building in shared library mode
if(NOT BUILD_SHARED_LIBS)
  target_link_libraries("${APP_NAME}" ${MW_PLUGINS})
endif(NOT BUILD_SHARED_LIBS)

# This ensure that the paraview.qch file is generated.
add_dependencies(${paraview_client_real_exe_name}
  ParaViewOnlineHelp)

# add include directories
include_directories(${PARAVIEW_INCLUDE_DIRS} ${PARAVIEW_APPLICATION_DIR})

target_link_libraries(MADAIWorkbench
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  vtkCommon
  vtkPVCommon
  pqCore
  pqComponents
)

# Setup install directories.
IF(NOT PV_INSTALL_BIN_DIR)
  SET(PV_INSTALL_BIN_DIR bin)
ENDIF(NOT PV_INSTALL_BIN_DIR)
IF(NOT PV_INSTALL_INCLUDE_DIR)
  SET(PV_INSTALL_INCLUDE_DIR include/paraview-${PARAVIEW_VERSION})
ENDIF(NOT PV_INSTALL_INCLUDE_DIR)
IF(NOT PV_INSTALL_LIB_DIR)
  SET(PV_INSTALL_LIB_DIR lib/paraview-${PARAVIEW_VERSION})
ENDIF(NOT PV_INSTALL_LIB_DIR)
IF(NOT PV_INSTALL_PLUGIN_DIR)
  IF(WIN32)
    SET(PV_INSTALL_PLUGIN_DIR ${PV_INSTALL_BIN_DIR})
  ELSE(WIN32)
    SET(PV_INSTALL_PLUGIN_DIR ${PV_INSTALL_LIB_DIR})
  ENDIF(WIN32)
ENDIF(NOT PV_INSTALL_PLUGIN_DIR)
IF(NOT PV_INSTALL_DATA_DIR)
  SET(PV_INSTALL_DATA_DIR share/paraview-${PARAVIEW_VERSION})
ENDIF(NOT PV_INSTALL_DATA_DIR)
IF(NOT PV_INSTALL_CMAKE_DIR)
  SET(PV_INSTALL_CMAKE_DIR ${PV_INSTALL_LIB_DIR}/CMake)
ENDIF(NOT PV_INSTALL_CMAKE_DIR)
IF(NOT PV_INSTALL_DOC_DIR)
  SET(PV_INSTALL_DOC_DIR share/doc/paraview-${PARAVIEW_VERSION})
ENDIF(NOT PV_INSTALL_DOC_DIR)

#------------------------------------------------------------------------
# Import site specific installation rules file, if any.
set (PARAVIEW_EXTRA_INSTALL_RULES_FILE "" CACHE FILEPATH
  "Path to site-specific install rules for ParaView.")
mark_as_advanced(PARAVIEW_EXTRA_INSTALL_RULES_FILE)

if (EXISTS "${PARAVIEW_EXTRA_INSTALL_RULES_FILE}")
  foreach(rule_file ${PARAVIEW_EXTRA_INSTALL_RULES_FILE})
    include("${rule_file}")
  endforeach(rule_file ${PARAVIEW_EXTRA_INSTALL_RULES_FILE})
endif (EXISTS "${PARAVIEW_EXTRA_INSTALL_RULES_FILE}")

#------------------------------------------------------------------------
# Install the .plugins configuration file.
install(FILES "${EXECUTABLE_OUTPUT_PATH}/.plugins"
        DESTINATION ${PV_INSTALL_PLUGIN_DIR}
        COMPONENT BrandedRuntime)

# Call this last so the install script runs after the above targets are installed:
#
if(APPLE)

  set (ParaView_BUNDLE_LOCATION "" CACHE STRING
  "Subdirectory in which to place MADAIWorkbench app inside a bundle.")

  mark_as_advanced(ParaView_BUNDLE_LOCATION)

  set(distributed_plugins ${PARAVIEW_PLUGINLIST})
  set(PV_EXE_NAME "${APP_NAME}")
  set(OSX_MAKE_STANDALONE_BUNDLE_CMAKE_SCRIPT
    "${CMAKE_CURRENT_BINARY_DIR}/MADAIWorkbench_OSX_MakeStandAloneBundle.cmake")
  set(OSX_MAKE_STANDALONE_BUNDLE_BASH_SCRIPT
    "${CMAKE_CURRENT_BINARY_DIR}/MADAIWorkbench_OSX_MakeStandAloneBundle.sh")

  set (QT_SQLLITE_PLUGINS_DIR ${QT_PLUGINS_DIR}/sqldrivers)

  # override settings from ParaView
  set(PARAVIEW_APPLICATION_NAME "${APP_NAME}")
  set(PARAVIEW_VERSION_FULL "${MW_VERSION_STRING}")

  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/CompleteBundle.cmake.in"
    "${OSX_MAKE_STANDALONE_BUNDLE_CMAKE_SCRIPT}" @ONLY IMMEDIATE)
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/CreateBundle.sh.in"
    "${OSX_MAKE_STANDALONE_BUNDLE_BASH_SCRIPT}" @ONLY IMMEDIATE)

  install(SCRIPT "${OSX_MAKE_STANDALONE_BUNDLE_CMAKE_SCRIPT}" COMPONENT Bundle)
endif(APPLE)

#------------------------------------------------------------------------
# Install ParaView client
build_paraview_client_cpack_config_init(
    PACKAGE_NAME "MADAIWorkbench"
    ORGANIZATION "Modeling and Data Analysis Initiative"
    VERSION_MAJOR ${MW_VERSION_MAJOR}
    VERSION_MINOR ${MW_VERSION_MINOR}
    VERSION_PATCH ${MW_VERSION_PATCH}
    DESCRIPTION "MADAI Workbench is a scientific visualization tool"
    LICENSE_FILE "${ParaView_SOURCE_DIR}/License_v1.2.txt"
    DESCRIPTION_FILE "${ParaView_SOURCE_DIR}/License_v1.2.txt"
    PACKAGE_EXECUTABLES "madaiworkbench;MADAIWorkbench"
)

# This ensures that the only component that is used by cpack when building the
# drag-n-drop on Mac is the "Bundle" component.
set (CPACK_INSTALL_CMAKE_PROJECTS_DRAGNDROP
  "${CMAKE_CURRENT_BINARY_DIR}" "MADAIWorkbench Mac Bundle" "Bundle" "/")

build_paraview_client_cpack_config()
