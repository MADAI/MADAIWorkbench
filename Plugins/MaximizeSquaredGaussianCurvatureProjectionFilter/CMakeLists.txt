include_directories(../PercentileSurfaceFilter ../RemoveLargeTetrahedraFilter)

add_paraview_plugin(
  MaximizeSquaredGaussianCurvatureProjectionFilter "1.0"
  SERVER_MANAGER_XML MaximizeSquaredGaussianCurvatureProjectionFilter.xml
  SERVER_MANAGER_SOURCES vtkMaximizeSquaredGaussianCurvatureProjectionFilter.cxx
    ../PercentileSurfaceFilter/vtkPercentileSurfaceFilter.cxx
    ../RemoveLargeTetrahedraFilter/vtkRemoveLargeTetrahedraFilter.cxx
  GUI_RESOURCE_FILES MaximizeSquaredGaussianCurvatureProjectionFilterGUI.xml
)
