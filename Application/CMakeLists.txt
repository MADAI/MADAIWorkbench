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
    SPLASH_IMAGE "${PROJECT_SOURCE_DIR}/Application/MADAILogo.png"
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
    #REQUIRED_PLUGINS LogoPlugin
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


#build_paraview_client_cpack_config_init(
#    PACKAGE_NAME "MADAIWorkbench"
#    ORGANIZATION "Modeling and Data Analysis Initiative"
#    VERSION_MAJOR ${MW_VERSION_MAJOR}
#    VERSION_MINOR ${MW_VERSION_MINOR}
#    VERSION_PATCH ${MW_VERSION_PATCH}
#    DESCRIPTION "MADAI Workbench is a scientific visualization tool"
#    LICENSE_FILE "${ParaView_SOURCE_DIR}/License_v1.2.txt"
#    DESCRIPTION_FILE "${ParaView_SOURCE_DIR}/License_v1.2.txt"
#    PACKAGE_EXECUTABLES "madaiworkbench;MADAIWorkbench"
#)

# This ensure that the only component that is used by cpack when building the
# drag-n-drop on Mac is the "Bundle" component.
#set (CPACK_INSTALL_CMAKE_PROJECTS_DRAGNDROP
#  "${CMAKE_CURRENT_BINARY_DIR}" "MADAIWorkbench Mac Bundle" "Bundle" "/")

#build_paraview_client_cpack_config()

#------------------------------------------------------------------------
# CPack settings (installer)
#------------------------------------------------------------------------
# CPack variables
SET(APP_NAME "MADAIWorkbench")
SET(CPACK_PACKAGE_NAME               "${APP_NAME}")
SET(CPACK_PACKAGE_DESCRIPTION        "MADAIWorkbench")
SET(CPACK_PACKAGE_VENDOR             "Modeling and Data Analysis Initiative")
SET(CPACK_PACKAGE_VERSION_MAJOR      ${MW_VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR      ${MW_VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH      ${MW_VERSION_PATCH})
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "${CPACK_PACKAGE_NAME}")
#SET(CPACK_PACKAGE_DESCRIPTION_FILE  "${RESOURCES}/README.txt")
#SET(CPACK_RESOURCE_FILE_LICENSE     "${RESOURCES}/LICENSE.txt")

SET(CPACK_PACKAGE_EXECUTABLES "${APP_NAME};${APP_NAME}")

IF(APPLE)
  SET(CPACK_GENERATOR DragNDrop)

  INSTALL(TARGETS "${APP_NAME}" BUNDLE DESTINATION ".")

  SET(CPACK_PACKAGE_FILE_NAME      "${APP_NAME}-${MW_VERSION_MAJOR}.${MW_VERSION_MINOR}.${MW_VERSION_PATCH}")
  SET(CPACK_BUNDLE_ICON	           "${CMAKE_CURRENT_SOURCE_DIR}/MacIcon.icns")
  SET(CPACK_BUNDLE_NAME            "${APP_NAME}")
  SET(CPACK_BUNDLE_PLIST           "${CMAKE_BINARY_DIR}/bin/MADAIWorkbench.app/Contents/Info.plist")
  SET(CPACK_BUNDLE_STARTUP_COMMAND "${CMAKE_BINARY_DIR}/bin/MADAIWorkbench.app/Contents/MacOS/MADAIWorkbench")

  # The qt.conf file tells Qt to disregard hard-coded paths in the Qt libraries.
  FILE(WRITE "${CMAKE_BINARY_DIR}/bin/MADAIWorkbench.app/Contents/Resources/qt.conf" "")
  INSTALL(FILES "${CMAKE_BINARY_DIR}/bin/MADAIWorkbench.app/Contents/Resources/qt.conf"
    DESTINATION "MADAIWorkbench.app/Contents/Resources")

  INSTALL(CODE "
          INCLUDE(BundleUtilities)
          FIXUP_BUNDLE( \"${CMAKE_BINARY_DIR}/bin/${APP_NAME}.app\" \"\" \"\" )
          " COMPONENT Runtime )



ENDIF(APPLE)

# Creates a 'PACKAGE' project
INCLUDE(CPack)
