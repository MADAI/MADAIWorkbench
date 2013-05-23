add_paraview_plugin(
  RescalePointsFilter "1.0"
  SERVER_MANAGER_XML RescalePointsFilter.xml
  SERVER_MANAGER_SOURCES vtkRescalePointsFilter.cxx
  GUI_RESOURCE_FILES RescalePointsFilterGUI.xml
)
