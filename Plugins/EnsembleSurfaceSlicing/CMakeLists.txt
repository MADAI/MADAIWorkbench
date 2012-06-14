add_paraview_plugin(
  EnsembleSurfaceSlicing "1.0"
  SERVER_MANAGER_XML EnsembleSurfaceSlicing.xml
  SERVER_MANAGER_SOURCES
    vtkEnsembleSurfaceSlicingActor.cxx
    vtkEnsembleSurfaceSlicingRepresentation.cxx
  )
