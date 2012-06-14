/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkEnsembleSurfaceSlicingPolyDataMapper.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkEnsembleSurfaceSlicingPolyDataMapper.h"

#include "vtkCompositeDataIterator.h"
#include "vtkCompositeDataPipeline.h"
#include "vtkCompositeDataSet.h"
#include "vtkExecutive.h"
#include "vtkGarbageCollector.h"
#include "vtkInformation.h"
#include "vtkMapper.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkOpenGLRenderWindow.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkRenderer.h"
#include "vtkShader2.h"
#include "vtkShader2Collection.h"
#include "vtkShaderProgram2.h"
#include "vtkUniformVariables.h"

#include <vector>

vtkStandardNewMacro(vtkEnsembleSurfaceSlicingPolyDataMapper);

static const char* vertexShader =
  "#version 120\n"
  "varying vec4 position;\n"
  "varying vec3 normal;\n"
  "varying vec3 vertexToLightVector;\n"
  "\n"
  "void main(void)\n"
  "{\n"
  "  normal = gl_NormalMatrix * gl_Normal;\n"
  "  vec4 vertexInModelViewSpace = gl_ModelViewMatrix * gl_Vertex;\n"
  "  vertexToLightVector = vec3( vertexInModelViewSpace.xyz - gl_LightSource[0].position.xyz );\n"
  "  gl_FrontColor = gl_Color;\n"
  "  position = gl_Vertex;\n"
  "  gl_Position = ftransform();\n"
  "}\n";

static const char* fragmentShader =
  "#version 120\n"
  "varying vec4 position;\n"
  "varying vec3 normal;\n"
  "varying vec3 vertexToLightVector;\n"
  "uniform float sliceWidth;\n"
  "uniform float sliceOffset;\n"
  "\n"
  "void main()\n"
  "{\n"
  "  vec3 normalizedNormal = normalize(normal);\n"
  "  if (gl_FrontFacing)\n"
  "    normalizedNormal = -normalizedNormal;\n"
  "  vec3 normalizedVertexToLightVector = normalize(vertexToLightVector);\n"
  "  float diffuseTerm = dot(normalizedNormal, normalizedVertexToLightVector);\n"
  "  diffuseTerm = clamp(diffuseTerm, 0.0, 1.0);\n"
  "  gl_FragColor.rgb = gl_Color.rgb * diffuseTerm;\n"
  "  gl_FragColor.a = 1.0;\n"
  //"  if ( step(sliceWidth, mod(position.z + sliceOffset, 1.0)) > 0.0 )\n"
  "  float modPosition = mod(position.z, 1.0);\n"
  "  if ( modPosition < sliceOffset || modPosition >= sliceOffset + sliceWidth )\n"
  "    {\n"
  "    discard;\n"
  "    }\n"
  "}\n";


// Definition brought over from the superclass
class vtkCompositePolyDataMapperInternals
{
public:
  std::vector<vtkPolyDataMapper*> Mappers;
};


vtkEnsembleSurfaceSlicingPolyDataMapper::vtkEnsembleSurfaceSlicingPolyDataMapper()
{
  this->IsInitialized = 0;
  this->RenderWindow = NULL;
}

vtkEnsembleSurfaceSlicingPolyDataMapper::~vtkEnsembleSurfaceSlicingPolyDataMapper()
{
  if ( this->VertexShader.GetPointer() )
    {
    this->VertexShader->ReleaseGraphicsResources();
    }

  if ( this->FragmentShader.GetPointer() )
    {
    this->FragmentShader->ReleaseGraphicsResources();
    }

  if ( this->Program.GetPointer() )
    {
    this->Program->ReleaseGraphicsResources();
    }
}

void vtkEnsembleSurfaceSlicingPolyDataMapper::Render(vtkRenderer *ren, vtkActor *a)
{
  //If the PolyDataMappers are not up-to-date then rebuild them
  vtkCompositeDataPipeline * executive = 
    vtkCompositeDataPipeline::SafeDownCast(this->GetExecutive());
  
  if(executive->GetPipelineMTime() > this->InternalMappersBuildTime.GetMTime())
    {
    this->BuildPolyDataMapper();    
    }

  if ( !this->IsInitialized )
    {
    this->RenderWindow = vtkOpenGLRenderWindow::SafeDownCast( ren->GetRenderWindow() );
    if ( !this->RenderWindow )
      {
      vtkErrorMacro( << "render window must be an vtkOpenGLRenderWindow");
      return;
      }
    }

  this->Initialize();

  // Set the slice width
  float sliceWidth = 1.0 / static_cast<float>( this->Internal->Mappers.size() );
  
  this->TimeToDraw = 0;
  //Call Render() on each of the PolyDataMappers
  for(unsigned int i=0;i<this->Internal->Mappers.size();i++)
    {
    if ( this->ClippingPlanes != 
         this->Internal->Mappers[i]->GetClippingPlanes() )
      {
      this->Internal->Mappers[i]->SetClippingPlanes( this->ClippingPlanes );
      }

    // Need to disable the shader program prior to changing the uniform values
    this->Program->Restore();

    // Set the slice offset
    float sliceOffset = sliceWidth * i;
    this->FragmentShader->GetUniformVariables()->SetUniformf("sliceWidth", 1, &sliceWidth);
    this->FragmentShader->GetUniformVariables()->SetUniformf("sliceOffset", 1, &sliceOffset);

    // Re-enable the shader program after the uniform values have been set
    this->Program->Use();
    
    this->Internal->Mappers[i]->SetLookupTable(
      this->GetLookupTable());
    this->Internal->Mappers[i]->SetScalarVisibility(
      this->GetScalarVisibility());
    this->Internal->Mappers[i]->SetUseLookupTableScalarRange(
      this->GetUseLookupTableScalarRange());
    this->Internal->Mappers[i]->SetScalarRange(
      this->GetScalarRange());
    this->Internal->Mappers[i]->SetImmediateModeRendering(
      this->GetImmediateModeRendering());
    this->Internal->Mappers[i]->SetColorMode(this->GetColorMode());
    this->Internal->Mappers[i]->SetInterpolateScalarsBeforeMapping(
      this->GetInterpolateScalarsBeforeMapping());

    this->Internal->Mappers[i]->SetScalarMode(this->GetScalarMode());
    if ( this->ScalarMode == VTK_SCALAR_MODE_USE_POINT_FIELD_DATA ||
         this->ScalarMode == VTK_SCALAR_MODE_USE_CELL_FIELD_DATA )
      {
      if ( this->ArrayAccessMode == VTK_GET_ARRAY_BY_ID )
        {
        this->Internal->Mappers[i]->ColorByArrayComponent(
          this->ArrayId,ArrayComponent);
        }
      else
        {
        this->Internal->Mappers[i]->ColorByArrayComponent(
          this->ArrayName,ArrayComponent);
        }
      }
  
    this->Internal->Mappers[i]->Render(ren,a);    
    this->TimeToDraw += this->Internal->Mappers[i]->GetTimeToDraw();
    }

  this->Cleanup();
}

int vtkEnsembleSurfaceSlicingPolyDataMapper::Initialize()
{
  if ( !IsInitialized )
    {
    this->VertexShader = vtkSmartPointer< vtkShader2 >::New();
    this->VertexShader->SetType( VTK_SHADER_TYPE_VERTEX );
    this->VertexShader->SetSourceCode( vertexShader );
    this->VertexShader->SetContext( this->RenderWindow );

    this->FragmentShader = vtkSmartPointer< vtkShader2 >::New();
    this->FragmentShader->SetType( VTK_SHADER_TYPE_FRAGMENT );
    this->FragmentShader->SetSourceCode( fragmentShader );
    this->FragmentShader->SetContext( this->RenderWindow );

    this->Program = vtkSmartPointer< vtkShaderProgram2 >::New();
    this->Program->SetContext( this->RenderWindow );
    this->Program->GetShaders()->AddItem( this->VertexShader );
    this->Program->GetShaders()->AddItem( this->FragmentShader );

    // Build the shader programs
    this->Program->Build();
    if ( this->Program->GetLastBuildStatus() != VTK_SHADER_PROGRAM2_LINK_SUCCEEDED )
      {
      vtkErrorMacro( "Couldn't build the vertex shader program." );
      return 0;
      }

    this->IsInitialized = 1;
    }

  this->Program->Use();

  return 1;
}

//----------------------------------------------------------------------------
int vtkEnsembleSurfaceSlicingPolyDataMapper::Cleanup()
{
  if ( this->Program.GetPointer() )
    {
    this->Program->Restore();
    }
  else
    {
    return 0;
    }

  return 1;
}
