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

#include "vtkSmartPointer.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkInformationVector.h"
#include "vtkInformation.h"
#include "vtkDataObject.h"
#include "vtkSmartPointer.h"
#include "vtkPoints.h"
#include "vtkPointSet.h"

#include "vtkRescalePointsFilter.h"

vtkStandardNewMacro(vtkRescalePointsFilter)

vtkRescalePointsFilter::vtkRescalePointsFilter()
{
  this->SetNumberOfInputPorts(1);
  this->SetNumberOfOutputPorts(1);

  // Default to 1x1x1 box
  this->OutputBounds[0] = 0.0;
  this->OutputBounds[1] = 1.0;
  this->OutputBounds[2] = 0.0;
  this->OutputBounds[3] = 1.0;
  this->OutputBounds[4] = 0.0;
  this->OutputBounds[5] = 1.0;
}

vtkRescalePointsFilter::~vtkRescalePointsFilter()
{
}

int vtkRescalePointsFilter::RequestData(
    vtkInformation *vtkNotUsed(request),
    vtkInformationVector **inputVector,
    vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and ouptut
  vtkPointSet *input = vtkPointSet::SafeDownCast(
      inInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkPointSet *output = vtkPointSet::SafeDownCast(
      outInfo->Get(vtkDataObject::DATA_OBJECT()));

  // We can shallow copy most of the input object because this filter
  // only changes the point positions.
  output->ShallowCopy(input);

  // We need to deep copy the points to avoid modifying the input
  // points.
  vtkPoints * points = vtkPoints::New();
  points->DeepCopy( output->GetPoints() );
  output->SetPoints( points );
  points->Delete();

  double inputBounds[6];
  points->GetBounds(inputBounds);

  double inputInverseRange[3];
  double outputRange[3];

  for (int i = 0; i < 3; ++i)
    {
    inputInverseRange[i] = inputBounds[2*i + 1] - inputBounds[2*i];
    if (inputInverseRange[i] != 0.0)
      {
      inputInverseRange[i] = 1.0 / inputInverseRange[i];
      }

    outputRange[i] = this->OutputBounds[2*i + 1] - this->OutputBounds[2*i];
    }

  vtkIdType numberOfPoints = points->GetNumberOfPoints();
  for (vtkIdType id = 0; id < numberOfPoints; ++id) {
    double xyz[3];
    points->GetPoint(id, xyz);
    for (int j = 0; j < 3; ++j)
      {
      xyz[j] = ((xyz[j] - inputBounds[2*j]) * inputInverseRange[j]) *
        outputRange[j] + this->OutputBounds[2*j];
      }
    points->SetPoint(id, xyz);
  }
  return 1;
}


//----------------------------------------------------------------------------
void vtkRescalePointsFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "OutputBounds: ["
     << this->OutputBounds[0] << ", "
     << this->OutputBounds[1] << ", "
     << this->OutputBounds[2] << ", "
     << this->OutputBounds[3] << ", "
     << this->OutputBounds[4] << ", "
     << this->OutputBounds[5] << "]\n";
}
