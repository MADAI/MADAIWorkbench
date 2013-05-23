include_directories( ../RemoveLargeTetrahedraFilter )

add_paraview_plugin(
  PercentileSurfaceFilter "1.0"
  SERVER_MANAGER_XML PercentileSurfaceFilter.xml
  SERVER_MANAGER_SOURCES vtkPercentileSurfaceFilter.cxx
    ../RemoveLargeTetrahedraFilter/vtkRemoveLargeTetrahedraFilter.cxx
  GUI_RESOURCE_FILES PercentileSurfaceFilterGUI.xml
)
