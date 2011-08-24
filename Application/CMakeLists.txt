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

# List the plugins that will be installed and available in the workbench
set(MW_PLUGINS
  LogoPlugin
  BinningFilter
  GaussianScalarSplatter
  BooleanOperation
)

set(APP_NAME "MADAIWorkbench")
build_paraview_client("${APP_NAME}"
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
    REQUIRED_PLUGINS ${MW_PLUGINS}
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
set(CPACK_PACKAGE_NAME               "${APP_NAME}")
set(CPACK_PACKAGE_DESCRIPTION        "MADAIWorkbench")
set(CPACK_PACKAGE_VENDOR             "Modeling and Data Analysis Initiative")
set(CPACK_PACKAGE_VERSION_MAJOR      ${MW_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR      ${MW_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH      ${MW_VERSION_PATCH})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "${CPACK_PACKAGE_NAME}")
#SET(CPACK_PACKAGE_DESCRIPTION_FILE  "${RESOURCES}/README.txt")
#SET(CPACK_RESOURCE_FILE_LICENSE     "${RESOURCES}/LICENSE.txt")

set(CPACK_PACKAGE_EXECUTABLES "${APP_NAME};${APP_NAME}")

if(APPLE)
  set(CPACK_GENERATOR DragNDrop)

  install(TARGETS "${APP_NAME}" BUNDLE DESTINATION ".")

  set(CPACK_PACKAGE_FILE_NAME      "${APP_NAME}-${MW_VERSION_MAJOR}.${MW_VERSION_MINOR}.${MW_VERSION_PATCH}")
  set(CPACK_BUNDLE_ICON	           "${CMAKE_CURRENT_SOURCE_DIR}/MacIcon.icns")
  set(CPACK_BUNDLE_NAME            "${APP_NAME}")
  set(CPACK_BUNDLE_PLIST           "${CMAKE_BINARY_DIR}/bin/MADAIWorkbench.app/Contents/Info.plist")
  set(CPACK_BUNDLE_STARTUP_COMMAND "${CMAKE_BINARY_DIR}/bin/MADAIWorkbench.app/Contents/MacOS/MADAIWorkbench")

  # Copy other ParaView executables into the bundle
  add_custom_command(TARGET "${APP_NAME}" POST_BUILD
    COMMAND "${CMAKE_SOURCE_DIR}/Application/CopyPVApplications.sh"
            "${ParaView_DIR}" "${CMAKE_BINARY_DIR}/bin/MADAIWorkbench.app"
    VERBATIM)

  # The qt.conf file tells Qt to disregard hard-coded paths in the Qt libraries
  execute_process( COMMAND "touch ${CMAKE_BINARY_DIR}/bin/MADAIWorkbench.app/Contents/Resources/qt.conf")
  install(FILES "${CMAKE_BINARY_DIR}/bin/MADAIWorkbench.app/Contents/Resources/qt.conf"
    DESTINATION "MADAIWorkbench.app/Contents/Resources")

  # Copy python files
  add_custom_command(TARGET "${APP_NAME}" POST_BUILD
    COMMAND "${CMAKE_SOURCE_DIR}/Application/CopyPython.sh"
            "${ParaView_DIR}" "${CMAKE_BINARY_DIR}/bin/MADAIWorkbench.app"
    VERBATIM)

  # Copy custom plugins into the bundle.
  add_custom_command(TARGET "${APP_NAME}" POST_BUILD
    COMMAND "${CMAKE_SOURCE_DIR}/Application/CopyPVPlugins.sh"
            "${CMAKE_BINARY_DIR}" "${CMAKE_BINARY_DIR}/bin/MADAIWorkbench.app"
    VERBATIM)

#  install(CODE "
#    INCLUDE(BundleUtilities)
#    FIXUP_BUNDLE( \"${CMAKE_BINARY_DIR}/bin/${APP_NAME}.app\" \"\" \"\" )
#    " COMPONENT Runtime )

  FILE(GLOB PV_PLUGIN_DYLIBS "${CMAKE_BINARY_DIR}/bin/${APP_NAME}.app/Contents/Plugins/*.dylib")
  MESSAGE( "PV Plugin List: ${PV_PLUGIN_DYLIBS}" )

  # Fixup the ParaView plugins
  install(CODE "
    FILE(GLOB PV_PLUGIN_DYLIBS \"${CMAKE_BINARY_DIR}/bin/${APP_NAME}.app/Contents/Plugins/*.dylib\")
    INCLUDE(BundleUtilities)
    FIXUP_BUNDLE(\"${CMAKE_BINARY_DIR}/bin/${APP_NAME}.app\"
      \"${PV_PLUGIN_DYLIBS}\" \"${CMAKE_BINARY_DIR}/bin/${APP_NAME}.app/Contents/Plugins\")
    " COMPONENT BrandedRuntime)

  # Generate the plugins XML configuration file.
  set(plugins_ini "<?xml version=\"1.0\"?>\n<Plugins>\n")
  foreach(plugin_name "${MW_PLUGINS}")
    set(plugins_ini "${plugins_ini} <Plugin name=\"${plugin_name}\" auto_load=\"1\"/>\n")
  endforeach()
  set(plugins_ini "${plugins_ini}</Plugins>\n")

  file(WRITE "${EXECUTABLE_OUTPUT_PATH}/.plugins" "${plugins_ini}")

  # Install the .plugins configuration file.
  install(FILES "${EXECUTABLE_OUTPUT_PATH}/.plugins"
    DESTINATION "MADAIWorkbench.app/Contents/Support"
    COMPONENT BrandedRuntime)

endif(APPLE)

# Creates a 'PACKAGE' project
include(CPack)
