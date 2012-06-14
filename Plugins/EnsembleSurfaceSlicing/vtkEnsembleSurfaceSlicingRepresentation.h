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

  void SetSliceWidth(float width);
  vtkGetMacro(SliceWidth, float);

//BTX
protected:
  vtkEnsembleSurfaceSlicingRepresentation();
  ~vtkEnsembleSurfaceSlicingRepresentation();

private:
  vtkEnsembleSurfaceSlicingRepresentation(const vtkEnsembleSurfaceSlicingRepresentation&); // Not implemented
  void operator=(const vtkEnsembleSurfaceSlicingRepresentation&); // Not implemented
//ETX

  float SliceWidth;
};

#endif
