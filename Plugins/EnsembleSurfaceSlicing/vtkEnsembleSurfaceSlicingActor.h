/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkEnsembleSurfaceSlicingActor.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkEnsembleSurfaceSlicingActor - generates texture coords with a shader
// .SECTION Description
// OpenGL actor that generates texture coordinates from a
// transformation on the point positions in the dataset

#ifndef __vtkEnsembleSurfaceSlicingActor_h
#define __vtkEnsembleSurfaceSlicingActor_h

#include "vtkPVLODActor.h"

#include "vtkSmartPointer.h"

class vtkOpenGLRenderer;
class vtkOpenGLRenderWindow;
class vtkRenderWindow;
class vtkShader2;
class vtkShaderProgram2;

class vtkEnsembleSurfaceSlicingActor : public vtkPVLODActor
{
protected:

public:
  static vtkEnsembleSurfaceSlicingActor *New();
  vtkTypeMacro(vtkEnsembleSurfaceSlicingActor,vtkPVLODActor);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Uniform scale factor applied to texture coordinates
  vtkSetMacro(TextureScale, float);
  vtkGetMacro(TextureScale, float);

  // Description:
  // Actual actor render method.
  void Render(vtkRenderer *ren, vtkMapper *mapper);

  // Description:
  // We are making the input always have translucent geometry.
  int HasTranslucentPolygonalGeometry()
  {
    return 1;
  }

  // Description:
  // No, this actor is not opaque


protected:
  vtkEnsembleSurfaceSlicingActor();
  ~vtkEnsembleSurfaceSlicingActor();

private:
  vtkEnsembleSurfaceSlicingActor(const vtkEnsembleSurfaceSlicingActor&);  // Not implemented.
  void operator=(const vtkEnsembleSurfaceSlicingActor&);  // Not implemented.

  float TextureScale;

  int Initialize();

  int Cleanup();

  int IsInitialized;

  vtkOpenGLRenderWindow * RenderWindow;

  vtkSmartPointer< vtkShader2 > VertexShader;

  vtkSmartPointer< vtkShader2 > FragmentShader;

  vtkSmartPointer< vtkShaderProgram2 > Program;
};

#endif
