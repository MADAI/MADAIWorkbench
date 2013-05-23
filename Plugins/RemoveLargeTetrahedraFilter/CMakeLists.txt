add_paraview_plugin(
  RemoveLargeTetrahedraFilter "1.0"
  SERVER_MANAGER_XML RemoveLargeTetrahedraFilter.xml
  SERVER_MANAGER_SOURCES vtkRemoveLargeTetrahedraFilter.cxx
  GUI_RESOURCE_FILES RemoveLargeTetrahedraFilterGUI.xml
)
