/*=========================================================================

  Program:   ParaView
  Module:    vtkEnsembleSurfaceSlicingRepresentation.cxx

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkEnsembleSurfaceSlicingRepresentation.h"

#include "vtkEnsembleSurfaceSlicingActor.h"
#include "vtkEnsembleSurfaceSlicingPolyDataMapper.h"
#include "vtkCompositePolyDataMapper2.h"
#include "vtkInformation.h"
#include "vtkObjectFactory.h"
#include "vtkProperty.h"
#include "vtkPVLODActor.h"
#include "vtkPVRenderView.h"
#include "vtkRenderer.h"

vtkStandardNewMacro(vtkEnsembleSurfaceSlicingRepresentation);
//----------------------------------------------------------------------------
vtkEnsembleSurfaceSlicingRepresentation::vtkEnsembleSurfaceSlicingRepresentation()
{
  this->SliceWidth = 3.0;

  vtkEnsembleSurfaceSlicingPolyDataMapper *mapper = vtkEnsembleSurfaceSlicingPolyDataMapper::New();
  mapper->SetSliceWidth( this->SliceWidth );
  this->Mapper = mapper;

  this->BackfaceActor = vtkPVLODActor::New();
  this->BackfaceProperty = vtkProperty::New();
  this->BackfaceMapper = vtkCompositePolyDataMapper2::New();
  this->LODBackfaceMapper = vtkCompositePolyDataMapper2::New();
  this->BackfaceRepresentation = FOLLOW_FRONTFACE;

  // Since we are overriding SetupDefaults(), we need to call it again.
  this->SetupDefaults();
}

//----------------------------------------------------------------------------
vtkEnsembleSurfaceSlicingRepresentation::~vtkEnsembleSurfaceSlicingRepresentation()
{
}

//----------------------------------------------------------------------------
void vtkEnsembleSurfaceSlicingRepresentation::SetSliceWidth(double width)
{
  vtkEnsembleSurfaceSlicingPolyDataMapper *mapper =
    vtkEnsembleSurfaceSlicingPolyDataMapper::SafeDownCast( this->Mapper );
  if ( mapper && mapper->GetSliceWidth() != width )
    {
    this->SliceWidth = width;
    mapper->SetSliceWidth(static_cast<float>(width));
    this->Modified();
    }
}
