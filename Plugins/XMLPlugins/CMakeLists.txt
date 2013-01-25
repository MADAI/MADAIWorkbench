add_paraview_plugin(
  BooleanOperationFilter "1.0"
  SERVER_MANAGER_XML BooleanOperation.xml
  GUI_RESOURCE_FILES BooleanOperationGUI.xml
  )

add_paraview_plugin(
  ThresholdPointsFilter "1.0"
  SERVER_MANAGER_XML ThresholdPoints.xml
  GUI_RESOURCE_FILES ThresholdPointsGUI.xml
  )
