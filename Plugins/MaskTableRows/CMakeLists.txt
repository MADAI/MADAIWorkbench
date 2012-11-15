add_paraview_plugin(
  MaskTableRows "1.0"
  SERVER_MANAGER_XML MaskTableRows.xml
  SERVER_MANAGER_SOURCES vtkMaskTableRows.cxx
  GUI_RESOURCE_FILES MaskTableRowsGUI.xml
  )


# TODO - remove the line below when ParaView is fixed
target_link_libraries( MaskTableRows vtkRenderingMatplotlib )
