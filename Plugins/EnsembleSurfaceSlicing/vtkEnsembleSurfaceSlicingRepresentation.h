/*=========================================================================

  Program:   ParaView
  Module:    vtkEnsembleSurfaceSlicingRepresentation.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkEnsembleSurfaceSlicingRepresentation
// .SECTION Description
// vtkEnsembleSurfaceSlicingRepresentation extends vtkGeometryRepresentation to add
// support for rendering back and front faces separately (with different
// visibility and properties).

#ifndef __vtkEnsembleSurfaceSlicingRepresentation_h
#define __vtkEnsembleSurfaceSlicingRepresentation_h

#include "vtkGeometryRepresentationWithFaces.h"

class VTK_EXPORT vtkEnsembleSurfaceSlicingRepresentation : public vtkGeometryRepresentationWithFaces
{
public:
  static vtkEnsembleSurfaceSlicingRepresentation* New();
  vtkTypeMacro(vtkEnsembleSurfaceSlicingRepresentation, vtkGeometryRepresentationWithFaces);

  // Description:
  // Width of each slice, specified in world coordinates
  vtkSetMacro(SliceWidth, double);
  vtkGetMacro(SliceWidth, double);

  // Description:
  // Displacement of each slice, specified in world coordinates
  vtkSetMacro(SliceDisplacement, double);
  vtkGetMacro(SliceDisplacement, double);

  // Description:
  // Plane with which the slices should be parallel
  vtkSetVector3Macro(PlaneNormal, double);
  vtkGetVector3Macro(PlaneNormal, double);

//BTX
protected:
  vtkEnsembleSurfaceSlicingRepresentation();
  ~vtkEnsembleSurfaceSlicingRepresentation();

  // Description:
  // Passes on parameters to vtkProperty and vtkMapper
  virtual void UpdateColoringParameters();

  double SliceWidth;
  double SliceDisplacement;
  double PlaneNormal[3];

private:
  vtkEnsembleSurfaceSlicingRepresentation(const vtkEnsembleSurfaceSlicingRepresentation&); // Not implemented
  void operator=(const vtkEnsembleSurfaceSlicingRepresentation&); // Not implemented
//ETX

};

#endif
