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

/* TODO - make the following shaders mimic the lighting done with the
* standard Surface representation. */
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
  "varying vec4  position;\n"
  "varying vec3  normal;\n"
  "varying vec3  vertexToLightVector;\n"
  "uniform float sliceWidth;\n"
  "uniform float sliceDisplacement;\n"
  "uniform float sliceFraction;\n"
  "uniform float sliceOffset;\n"
  "uniform vec3  sliceNormal;\n"
  "uniform vec3  sliceColor;\n"
  "\n"
  "void main()\n"
  "{\n"
  "  vec3 normalizedNormal = normalize(normal);\n"
  "  if (gl_FrontFacing)\n"
  "    normalizedNormal = -normalizedNormal;\n"
  "  vec3 normalizedVertexToLightVector = normalize(vertexToLightVector);\n"
  "  float diffuseTerm = dot(normalizedNormal, normalizedVertexToLightVector);\n"
  "  diffuseTerm = clamp(diffuseTerm, 0.0, 1.0);\n"
  "  gl_FragColor.rgb = sliceColor * diffuseTerm;\n"
  "  gl_FragColor.a = 1.0;\n"
  "  float distance = dot( sliceNormal, position.xyz );\n"
  "  float modPosition = mod((distance-sliceDisplacement)*sliceFraction/sliceWidth, 1.0);\n"
  "  if ( modPosition < sliceOffset || modPosition >= sliceOffset + sliceFraction )\n"
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

// Set up color table used to determine which colors each surface should have
float vtkEnsembleSurfaceSlicingPolyDataMapper::SliceColors[3*(8*9)/2] = {
  0.250049,0.395637,0.0580882, //  64, 101,  15

  0.250049,0.395637,0.0580882, //  64, 101,  15
  0.44049,0.255049,0.628382,   // 112, 65, 160

  0.250049,0.395637,0.0580882, //  64, 101,  15
  0.0910785,0.372206,0.692451, //  23,  95, 177
  0.691127,0.209363,0.251078,  // 176,  53,  64

  0.250049,0.395637,0.0580882, //  64, 101,  15
  0.0230392,0.41598,0.542745,  //   6, 106, 138
  0.44049,0.255049,0.628382,   // 112,  65, 160
  0.684902,0.227843,0.129608,  // 175,  58,  33

  0.250049,0.395637,0.0580882, //  64, 101,  15
  0.0305882,0.42902,0.428627,  //   8, 109, 109
  0.211569,0.324069,0.729069,  //  54,  83, 186
  0.651912,0.212794,0.354216,  // 166,  54,  90
  0.623775,0.255637,0.0740196, // 159,  65,  19

  0.250049,0.395637,0.0580882, //  64, 101,  15
  0.0454412,0.431324,0.356765, //  12, 110,  91
  0.0910785,0.372206,0.692451, //  23,  95, 177
  0.44049,0.255049,0.628382,   // 112,  65, 160
  0.691127,0.209363,0.251078,  // 176,  53,  64
  0.570294,0.278431,0.0463726, // 145,  71,  12

  0.250049,0.395637,0.0580882, //  64, 101,  15
  0.0572759,0.431373,0.317745, //  15, 110,  81
  0.0380182,0.400973,0.625371, //  10, 102, 159
  0.288151,0.299461,0.716127,  //  73,  76, 183
  0.625588,0.215014,0.408011,  // 160,  55, 104
  0.701926,0.214741,0.178333,  // 179,  55,  45
  0.528417,0.296639,0.0336275,

  0.250049,0.395637,0.0580882, //  64, 101,  15
  0.066152,0.431373,0.291324,  //  17, 110,  74
  0.0230392,0.41598,0.542745,  //   6, 106, 138
  0.162966,0.343627,0.723113,  //  42,  88, 184
  0.44049,0.255049,0.628382,   // 112,  65, 160
  0.669265,0.210025,0.312059,  // 171,  54,  80
  0.684902,0.227843,0.129608,  // 175,  58,  33
  0.493039,0.308971,0.0253921};// 126,  53,   6

vtkEnsembleSurfaceSlicingPolyDataMapper::vtkEnsembleSurfaceSlicingPolyDataMapper()
{
  this->IsInitialized = 0;
  this->RenderWindow = NULL;

  this->SliceWidth = 0.25;
  this->SliceDisplacement = 0.0;

  this->PlaneNormal[0] = 1.0;
  this->PlaneNormal[1] = 0.0;
  this->PlaneNormal[2] = 0.0;
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

  // Set the slice fraction
  float sliceFraction = 1.0 / static_cast<float>( this->Internal->Mappers.size() );
  
  this->TimeToDraw = 0;
  //Call Render() on each of the PolyDataMappers
  size_t numMappers = this->Internal->Mappers.size();
  for(size_t i=0;i<numMappers;i++)
    {
    if ( this->ClippingPlanes != 
         this->Internal->Mappers[i]->GetClippingPlanes() )
      {
      this->Internal->Mappers[i]->SetClippingPlanes( this->ClippingPlanes );
      }

    // Need to disable the shader program prior to changing the uniform values
    this->Program->Restore();

    // Set the slice offset
    float sliceOffset = sliceFraction * i;
    vtkUniformVariables *uniforms = this->FragmentShader->GetUniformVariables();
    uniforms->SetUniformf("sliceFraction", 1, &sliceFraction);

    float sliceWidth = static_cast<float>(this->SliceWidth);
    uniforms->SetUniformf("sliceWidth",        1, &sliceWidth);
    float sliceDisplacement = static_cast<float>(this->SliceDisplacement);
    uniforms->SetUniformf("sliceDisplacement", 1, &sliceDisplacement);
    uniforms->SetUniformf("sliceOffset",       1, &sliceOffset);

    float sliceNormal[3];
    sliceNormal[0] = static_cast<float>( this->PlaneNormal[0] );
    sliceNormal[1] = static_cast<float>( this->PlaneNormal[1] );
    sliceNormal[2] = static_cast<float>( this->PlaneNormal[2] );
    vtkMath::Normalize( sliceNormal );
    uniforms->SetUniformf("sliceNormal", 3, sliceNormal);

    size_t sliceColorIndex = 3*(((numMappers-1)*numMappers / 2) + i);
    float *sliceColor = SliceColors + sliceColorIndex;
    uniforms->SetUniformf("sliceColor", 3, sliceColor);

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
