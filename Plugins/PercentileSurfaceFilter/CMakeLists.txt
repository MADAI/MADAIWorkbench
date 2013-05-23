# We include the directory and source files for the
# RemoveLargeTetrahedraFilter for now. It would be
# better to make a single library for these classes,
# but that is harder to do.
include_directories( ../RemoveLargeTetrahedraFilter )

add_paraview_plugin(
  PercentileSurfaceFilter "1.0"
  SERVER_MANAGER_XML PercentileSurfaceFilter.xml
  SERVER_MANAGER_SOURCES vtkPercentileSurfaceFilter.cxx
    ../RemoveLargeTetrahedraFilter/vtkRemoveLargeTetrahedraFilter.cxx
  GUI_RESOURCE_FILES PercentileSurfaceFilterGUI.xml
)
