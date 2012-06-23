/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkEnsembleSurfaceSlicingPolyDataMapper.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkEnsembleSurfaceSlicingPolyDataMapper - a class that renders hierarchical polygonal data
// .SECTION Description
// This class uses a set of vtkPolyDataMappers to render input data
// which may be hierarchical. The input to this mapper may be
// either vtkPolyData or a vtkEnsembleSurfaceSlicingDataSet built from
// polydata. If something other than vtkPolyData is encountered,
// an error message will be produced.
// .SECTION see also
// vtkPolyDataMapper

#ifndef __vtkEnsembleSurfaceSlicingPolyDataMapper_h
#define __vtkEnsembleSurfaceSlicingPolyDataMapper_h

#include "vtkCompositePolyDataMapper.h"

#include "vtkSmartPointer.h"

class vtkPolyDataMapper;
class vtkInformation;
class vtkRenderer;
class vtkActor;
class vtkOpenGLRenderWindow;
class vtkEnsembleSurfaceSlicingPolyDataMapperInternals;
class vtkShader2;
class vtkShaderProgram2;


class VTK_EXPORT vtkEnsembleSurfaceSlicingPolyDataMapper : public vtkCompositePolyDataMapper
{

public:
  static vtkEnsembleSurfaceSlicingPolyDataMapper *New();
  vtkTypeMacro(vtkEnsembleSurfaceSlicingPolyDataMapper, vtkCompositePolyDataMapper);

  // Description:
  // Standard method for rendering a mapper. This method will be
  // called by the actor.
  void Render(vtkRenderer *ren, vtkActor *a);

  // Description:
  // Width of each slice, specified in world coordinates
  vtkSetMacro(SliceWidth, double);
  vtkGetMacro(SliceWidth, double);

  // Description:
  // Displacement of each slice, specified in world coordinates
  vtkSetMacro(SliceDisplacement, double);
  vtkGetMacro(SliceDisplacement, double);


protected:
  vtkEnsembleSurfaceSlicingPolyDataMapper();
  ~vtkEnsembleSurfaceSlicingPolyDataMapper();

private:
  vtkEnsembleSurfaceSlicingPolyDataMapper(const vtkEnsembleSurfaceSlicingPolyDataMapper&);  // Not implemented.
  void operator=(const vtkEnsembleSurfaceSlicingPolyDataMapper&);    // Not implemented.

  int Initialize();

  int Cleanup();

  int IsInitialized;

  vtkOpenGLRenderWindow * RenderWindow;

  vtkSmartPointer< vtkShader2 > VertexShader;

  vtkSmartPointer< vtkShader2 > FragmentShader;

  vtkSmartPointer< vtkShaderProgram2 > Program;

  //BTX
  // Linear color array. The first three elements are the RGB color if one
  // surface is to be displayed. The next three elements are the RGB color
  // of the first surface if two surfaces are to be displayed, and the
  // three elements after that are the RGB color of the second surface, and so
  // on for up to eight surfaces.
  static float SliceColors[3*(8*9)/2];
  //ETX

  double SliceWidth;

  double SliceDisplacement;
};

#endif
