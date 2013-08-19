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

#include <cmath>

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

  this->RescaleByStandardScore = 0; // false
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
  vtkSmartPointer < vtkPoints > points = vtkSmartPointer < vtkPoints >::New();
  points->DeepCopy( output->GetPoints() );
  output->SetPoints( points );
  vtkIdType numberOfPoints = points->GetNumberOfPoints();
  double scaleFactor[3];
  double addend[3];

  if (this->RescaleByStandardScore)
    {
    // calculate means.
    double sum[3] = {0.0, 0.0, 0.0};
    for (vtkIdType id = 0; id < numberOfPoints; ++id)
      {
      double xyz[3];
      points->GetPoint(id, xyz);
      for (int j = 0; j < 3; ++j)
        {
        sum[j] += xyz[j];
        }
      }
    double mean[3];
    for (int j = 0; j < 3; ++j)
      {
      mean[j] = sum[j] / numberOfPoints;
      }
    // calculate variances.
    double sumSquaredDeviation[3] = {0.0, 0.0, 0.0};
    for (vtkIdType id = 0; id < numberOfPoints; ++id)
      {
      double xyz[3];
      points->GetPoint(id, xyz);
      for (int j = 0; j < 3; ++j)
        {
        sumSquaredDeviation[j] += std::pow(xyz[j] - mean[j], 2);
        }
      }
    for (int j = 0; j < 3; ++j)
      {
      scaleFactor[j] =
        1.0 / std::sqrt(sumSquaredDeviation[j] / numberOfPoints);
      addend[j] = - mean[j] * scaleFactor[j];
      }
    }
  else // (NOT this->RescaleByStandardScore => scale by bounds)
    {
    double inputBounds[6];
    points->GetBounds(inputBounds);

    for (int i = 0; i < 3; ++i)
      {
      double inputRange = inputBounds[2*i + 1] - inputBounds[2*i];
      double outputRange = this->OutputBounds[2*i+1] - this->OutputBounds[2*i];
      if (inputRange != 0.0)
        {
        scaleFactor[i] = outputRange / inputRange;
        }
      else
        {
        scaleFactor[i] = 0.0;
        }
      addend[i] = this->OutputBounds[2*i] - (inputBounds[2*i] * scaleFactor[i]);
      }
    }
  for (vtkIdType id = 0; id < numberOfPoints; ++id)
    {
    double xyz[3];
    points->GetPoint(id, xyz);
    for (int j = 0; j < 3; ++j)
      {
      xyz[j] = (xyz[j] * scaleFactor[j]) + addend[j];
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
  os << indent << "RescaleByStandardScore: "
     << ((this->RescaleByStandardScore) ? "true" : "false") << '\n';
}
