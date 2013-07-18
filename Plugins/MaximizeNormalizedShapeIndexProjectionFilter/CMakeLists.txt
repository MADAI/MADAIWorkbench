include_directories(
  ../PercentileSurfaceFilter
  ../RescalePointsFilter
  ../RemoveLargeTetrahedraFilter
)

add_paraview_plugin(
  MaximizeNormalizedShapeIndexProjectionFilter "1.0"
  SERVER_MANAGER_XML MaximizeNormalizedShapeIndexProjectionFilter.xml
  SERVER_MANAGER_SOURCES vtkMaximizeNormalizedShapeIndexProjectionFilter.cxx
    ../PercentileSurfaceFilter/vtkPercentileSurfaceFilter.cxx
    ../RescalePointsFilter/vtkRescalePointsFilter.cxx
    ../RemoveLargeTetrahedraFilter/vtkRemoveLargeTetrahedraFilter.cxx
  GUI_RESOURCE_FILES MaximizeNormalizedShapeIndexProjectionFilterGUI.xml
)
