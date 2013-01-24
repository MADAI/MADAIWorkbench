qt4_wrap_cpp(MOC_SRCS pqEnsembleSurfaceSlicingDecorator.h)
qt4_wrap_ui(UI_SRCS pqEnsembleSurfaceSlicingDecorator.ui)

add_paraview_display_panel_decorator(
  IFACES IFACE_SRCS
  CLASS_NAME pqEnsembleSurfaceSlicingDecorator
  PANEL_TYPES
    pqDisplayProxyEditor
)

add_paraview_plugin(
  EnsembleSurfaceSlicing "1.0"
  GUI_INTERFACES ${IFACES}
  SERVER_MANAGER_XML
    EnsembleSurfaceSlicing.xml
  SOURCES
    pqEnsembleSurfaceSlicingDecorator.cxx
    ${MOC_SRCS}
    ${UI_SRCS}
    ${IFACE_SRCS}
  SERVER_MANAGER_SOURCES
    vtkEnsembleSurfaceSlicingRepresentation.cxx
    vtkEnsembleSurfaceSlicingPolyDataMapper.cxx
)
