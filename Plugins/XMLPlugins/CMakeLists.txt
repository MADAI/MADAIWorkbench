add_paraview_plugin(
  BooleanOperationFilter "1.0"
  SERVER_MANAGER_XML BooleanOperation.xml
  GUI_RESOURCE_FILES BooleanOperationGUI.xml
  )

# TODO - remove the line below when ParaView is fixed
target_link_libraries( BooleanOperationFilter vtkRenderingMatplotlib )
