/*=========================================================================
 *
 *  Copyright 2011-2013 The University of North Carolina at Chapel Hill
 *  All rights reserved.
 *
 *  Licensed under the MADAI Software License. You may obtain a copy of
 *  this license at
 *
 *         https://madai-public.cs.unc.edu/visualization/software-license/
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/
// Written 2013 Hal Canary

#include "vtkPercentileSurfaceFilter.h"

#include "vtkDataSetAttributes.h"
#include "vtkDataSetSurfaceFilter.h"
#include "vtkDelaunay3D.h"
#include "vtkErrorCode.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkUnstructuredGrid.h"
#include "vtkThresholdPoints.h"

#include "vtkRemoveLargeTetrahedraFilter.h"

#include <algorithm>
#include <vector>


vtkStandardNewMacro(vtkPercentileSurfaceFilter)

vtkPercentileSurfaceFilter::vtkPercentileSurfaceFilter()
{
  this->SetNumberOfInputPorts(1);
  this->SetNumberOfOutputPorts(1);
  this->Percentile = 0.95; // default values
  this->MaximumEdgeLength = 0.25;

  // by default process active point scalars
  this->SetInputArrayToProcess(
    0,0,0, vtkDataObject::FIELD_ASSOCIATION_POINTS,
    vtkDataSetAttributes::SCALARS);
}

vtkPercentileSurfaceFilter::~vtkPercentileSurfaceFilter()
{
}


int vtkPercentileSurfaceFilter::RequestData(
    vtkInformation *vtkNotUsed(request),
    vtkInformationVector **inputVector,
    vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and ouptut
  vtkDataSet *input = vtkDataSet::SafeDownCast(
      inInfo->Get(vtkDataObject::DATA_OBJECT()));
  if (input == NULL)
    {
    vtkErrorMacro(<<"No input found");
    return 1;
    }

  vtkPolyData *output = vtkPolyData::SafeDownCast(
      outInfo->Get(vtkDataObject::DATA_OBJECT()));
  if (output == NULL)
    {
    return 1;
    }

  // I update here so that I know that if something goes wrong, the
  // debugger will mention this line in the stack trace.
  vtkDataArray * inScalars= this->GetInputArrayToProcess(0,inputVector);
  if ( inScalars == NULL )
    {
    vtkErrorMacro(<<"No scalar data to threshold");
    return 1;
    }
  vtkSmartPointer < vtkDelaunay3D > delaunay3D =
    vtkSmartPointer < vtkDelaunay3D >::New();

  if (this->Percentile < 1.0)
    {
    vtkIdType numberOfTuples = inScalars->GetNumberOfTuples();
    std::vector< double > values(numberOfTuples);
    if (numberOfTuples == 0)
      {
      vtkErrorMacro(<<"Empty scalar array.");
      return 0;
      }
    for (vtkIdType id = 0; id < numberOfTuples; ++id)
      {
      values[id] = inScalars->GetComponent(id, 0);
      }
    std::sort(values.begin(), values.end());
    size_t number_kept = values.size() * (this->Percentile);
    // Only executed if (this->GetDebug() != 0)
    vtkDebugMacro(
      << "Keeping " << number_kept << " of " << values.size() << '\n');
    if (number_kept == 0)
      {
      vtkErrorMacro(<<"Error: No points kept.  Try increasing Percentile.");
      return 0;
      }
    vtkSmartPointer < vtkThresholdPoints > thresholdPoints =
      vtkSmartPointer < vtkThresholdPoints >::New();
    vtkDebugMacro(
      << "Threshold is " << values[values.size() - number_kept] << '\n');

    thresholdPoints->ThresholdByUpper(values[values.size() - number_kept]);
    thresholdPoints->SetInputData(input);
    thresholdPoints->Update();
    vtkDebugMacro( << "Number finally kept = "
                   << thresholdPoints->GetOutput()->GetNumberOfPoints());

    delaunay3D->SetInputConnection(thresholdPoints->GetOutputPort());
    }
  else
    {
    delaunay3D->SetInputData(input);
    }

  delaunay3D->DebugOff(); // It is very possible that this fails.
  delaunay3D->Update();
  if (delaunay3D->GetOutput() == NULL)
    {
    vtkErrorMacro(<<"delaunay3D failed.");
    return 1;
    }
  if ((delaunay3D->GetOutput()->GetNumberOfPoints() == 0) ||
      (delaunay3D->GetOutput()->GetNumberOfCells() == 0))
    {
    return 1;
    }

  vtkSmartPointer < vtkRemoveLargeTetrahedraFilter > tetraFilter =
    vtkSmartPointer < vtkRemoveLargeTetrahedraFilter >::New();
  tetraFilter->SetMaximumEdgeLength(this->MaximumEdgeLength);
  tetraFilter->SetInputConnection(delaunay3D->GetOutputPort());
  tetraFilter->Update();

  vtkSmartPointer < vtkDataSetSurfaceFilter > surfaceFilter =
    vtkSmartPointer < vtkDataSetSurfaceFilter >::New();
  surfaceFilter->SetInputConnection(tetraFilter->GetOutputPort());
  surfaceFilter->Update();

  output->ShallowCopy(surfaceFilter->GetOutput());

  return 1;
}


//----------------------------------------------------------------------------
void vtkPercentileSurfaceFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Percentile: " << this->Percentile << "\n";
  os << indent << "MaximumEdgeLength: " << this->MaximumEdgeLength << "\n";
}
