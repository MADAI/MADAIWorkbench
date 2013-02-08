add_paraview_plugin(
  VectorComparisonGlyphFilter "1.0"
  SERVER_MANAGER_XML VectorComparisonGlyphFilter.xml
  SERVER_MANAGER_SOURCES vtkVectorComparisonGlyphFilter.cxx
  GUI_RESOURCE_FILES VectorComparisonGlyphFilterGUI.xml
  )
