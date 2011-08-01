# Set the version number
set(MW_VERSION_MAJOR 1)
set(MW_VERSION_MINOR 0)
set(MW_VERSION_PATCH 0)
set(MW_VERSION_STRING "${MW_VERSION_MAJOR}.${MW_VERSION_MINOR}.${MW_VERSION_PATCH}")

# Now build the MADAI workbench as a branded ParaView
include(${ParaView_CMAKE_DIR}/ParaViewBranding.cmake)
include(${ParaView_CMAKE_DIR}/ParaViewPlugins.cmake)

get_filename_component(ParaView_SOURCE_DIR "${ParaView_CMAKE_DIR}/.." ABSOLUTE)
set(PARAVIEW_APP_DIR "${ParaView_SOURCE_DIR}/Applications/ParaView")

QT4_WRAP_CPP(MOC_BUILT_SOURCES
  ${PARAVIEW_APP_DIR}/ParaViewMainWindow.h)

QT4_WRAP_UI(UI_BUILT_SOURCES
  ${PARAVIEW_APP_DIR}/ParaViewMainWindow.ui
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
  ${PARAVIEW_APP_DIR}/ParaViewMainWindow.cxx
  ${PARAVIEW_APP_DIR}/ParaViewMainWindow.h
  ${MOC_BUILT_SOURCES}
  ${UI_BUILT_SOURCES})

build_paraview_client(MADAIWorkbench
    TITLE "MADAI Workbench ${MW_VERSION_STRING}"
    ORGANIZATION  "Modeling and Data Analysis Initiative"
    VERSION_MAJOR ${MW_VERSION_MAJOR}
    VERSION_MINOR ${MW_VERSION_MINOR}
    VERSION_PATCH ${MW_VERSION_PATCH}
    SPLASH_IMAGE "${PROJECT_SOURCE_DIR}/Application/MADAILogo.jpg"
    PVMAIN_WINDOW ParaViewMainWindow
    PVMAIN_WINDOW_INCLUDE ParaViewMainWindow.h
    BUNDLE_ICON   "${PROJECT_SOURCE_DIR}/Application/MacIcon.icns"
    APPLICATION_ICON  "${PROJECT_SOURCE_DIR}/Application/WinIcon.ico"
    GUI_CONFIGURATION_XMLS
      ${PARAVIEW_APP_DIR}/ParaViewSources.xml
      ${PARAVIEW_APP_DIR}/ParaViewFilters.xml
      ${PARAVIEW_APP_DIR}/ParaViewReaders.xml
      ${PARAVIEW_APP_DIR}/ParaViewWriters.xml
      ${PARAVIEW_EXTRA_GUI_RESOURCES} # Add any compiled in GUI configuration xmls
    COMPRESSED_HELP_FILE "${ParaView_BINARY_DIR}/Documentation/paraview.qch"
    SOURCES ${ParaView_SOURCE_FILES}
    REQUIRED_PLUGINS LogoPlugin
)

include_directories(${PARAVIEW_INCLUDE_DIRS} ${PARAVIEW_APP_DIR})

target_link_libraries(MADAIWorkbench
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  vtkCommon
  vtkPVCommon
  pqCore
  pqComponents
)

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

# This ensure that the only component that is used by cpack when building the
# drag-n-drop on Mac is the "Bundle" component.
set (CPACK_INSTALL_CMAKE_PROJECTS_DRAGNDROP
  "${CMAKE_CURRENT_BINARY_DIR}" "MADAIWorkbench Mac Bundle" "Bundle" "/")

build_paraview_client_cpack_config()
