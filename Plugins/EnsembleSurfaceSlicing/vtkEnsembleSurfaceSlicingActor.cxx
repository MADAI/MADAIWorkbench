/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkTextureCoordOpenGLActor.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkEnsembleSurfaceSlicingActor.h"

#include "vtkMapper.h"
#include "vtkMatrix4x4.h"
#include "vtkObjectFactory.h"
#include "vtkOpenGLRenderer.h"
#include "vtkProperty.h"

#include "vtkOpenGL.h"
#include "vtkOpenGLRenderWindow.h"
#include "vtkShader2.h"
#include "vtkShader2Collection.h"
#include "vtkShaderProgram2.h"
#include "vtkUniformVariables.h"

vtkStandardNewMacro(vtkEnsembleSurfaceSlicingActor);

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
  "  if ( step(0.5, mod(position.z, 1.0)) > 0.0 )\n"
  "    {\n"
  "    discard;\n"
  "    }\n"
  "}\n";

vtkEnsembleSurfaceSlicingActor::vtkEnsembleSurfaceSlicingActor()
{
  this->TextureScale = 1.0f;
  this->IsInitialized = 0;
  this->RenderWindow = NULL;
}

vtkEnsembleSurfaceSlicingActor::~vtkEnsembleSurfaceSlicingActor()
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

// Actual actor render method.
void vtkEnsembleSurfaceSlicingActor::Render(vtkRenderer *ren, vtkMapper *mapper)
{
  this->RenderWindow = vtkOpenGLRenderWindow::SafeDownCast( ren->GetRenderWindow() );

  // Set up the vertex shader that assigns texture coordinates
  this->Initialize( );

  // Call the Render() method from the superclass
  this->Superclass::Render( ren, mapper );

  // Tear down the vertex shader
  this->Cleanup();
}

//----------------------------------------------------------------------------
int vtkEnsembleSurfaceSlicingActor::Initialize()
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
int vtkEnsembleSurfaceSlicingActor::Cleanup()
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

//----------------------------------------------------------------------------
void vtkEnsembleSurfaceSlicingActor::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
