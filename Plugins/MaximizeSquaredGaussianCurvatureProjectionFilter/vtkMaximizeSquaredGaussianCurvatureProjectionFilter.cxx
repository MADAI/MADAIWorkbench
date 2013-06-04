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
// Written 2013 Hal Canary and Cory Quammen

#include <cmath>
#include <limits>
#include <map>
#include <string>

#include "vtkCurvatures.h"
#include "vtkDataObject.h"
#include "vtkFieldData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPercentileSurfaceFilter.h"
#include "vtkPointData.h"
#include "vtkRescalePointsFilter.h"
#include "vtkSmartPointer.h"
#include "vtkSmoothPolyDataFilter.h"
#include "vtkStdString.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkStringArray.h"
#include "vtkTable.h"
#include "vtkTableToPolyData.h"

#include "vtkMaximizeSquaredGaussianCurvatureProjectionFilter.h"

class vtkMaximizeSquaredGaussianCurvatureProjectionFilterInternal
{
public:
  std::map< std::string, bool > ActivePointArrays;
};


vtkStandardNewMacro(vtkMaximizeSquaredGaussianCurvatureProjectionFilter)

vtkMaximizeSquaredGaussianCurvatureProjectionFilter::vtkMaximizeSquaredGaussianCurvatureProjectionFilter()
{
  this->SetNumberOfOutputPorts(2);
  this->Percentile = 0.95;
  this->RetainLowestValues = 0;
  this->MaximumEdgeLength = 0.25;
  this->Internal = new vtkMaximizeSquaredGaussianCurvatureProjectionFilterInternal();
}

vtkMaximizeSquaredGaussianCurvatureProjectionFilter::~vtkMaximizeSquaredGaussianCurvatureProjectionFilter()
{
  delete this->Internal;
}

int vtkMaximizeSquaredGaussianCurvatureProjectionFilter::FillInputPortInformation(
  int vtkNotUsed(port), vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkTable");
  return 1;
}

/**
   \todo Document */
double getScore(
  const char * scalarName, vtkTable * table,
  int XColumnIndex, int YColumnIndex, int ZColumnIndex,
  double maximumEdgeLength, int retainLowestValues, double percentile) {

  vtkSmartPointer< vtkTableToPolyData > tableToPoints =
    vtkSmartPointer< vtkTableToPolyData >::New();
  tableToPoints->SetXColumnIndex(XColumnIndex);
  tableToPoints->SetYColumnIndex(YColumnIndex);
  tableToPoints->SetZColumnIndex(ZColumnIndex);
  tableToPoints->PreserveCoordinateColumnsAsDataArraysOn();
  tableToPoints->SetInputData( table );

  vtkSmartPointer< vtkRescalePointsFilter > rescaleFilter =
    vtkSmartPointer< vtkRescalePointsFilter >::New();
  rescaleFilter->SetInputConnection( tableToPoints->GetOutputPort() );

  vtkSmartPointer < vtkPercentileSurfaceFilter > percentileSurfaceFilter =
    vtkSmartPointer < vtkPercentileSurfaceFilter >::New();
  percentileSurfaceFilter->SetInputConnection(
      rescaleFilter->GetOutputPort());
  percentileSurfaceFilter->SetPercentile(percentile);
  percentileSurfaceFilter->SetRetainLowestValues(retainLowestValues);
  percentileSurfaceFilter->SetMaximumEdgeLength(maximumEdgeLength);
  percentileSurfaceFilter->
    SetInputArrayToProcess( 0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS,
                            scalarName );
  percentileSurfaceFilter->Update();

  double score = -std::numeric_limits< double >::infinity();

  if ((percentileSurfaceFilter->GetOutput()->GetNumberOfPoints() == 0) ||
      (percentileSurfaceFilter->GetOutput()->GetNumberOfPolys() == 0))
    {
    return score;
    }
  vtkSmartPointer < vtkSmoothPolyDataFilter > smoothPolyDataFilter =
    vtkSmartPointer < vtkSmoothPolyDataFilter >::New();
  smoothPolyDataFilter->SetInputConnection(
    percentileSurfaceFilter->GetOutputPort());
  smoothPolyDataFilter->Update();

  if (smoothPolyDataFilter->GetOutput() == NULL ||
      smoothPolyDataFilter->GetOutput()->GetNumberOfPoints() == 0 ||
      smoothPolyDataFilter->GetOutput()->GetNumberOfPolys() == 0)
    {
    return score;
    }

  vtkSmartPointer < vtkCurvatures > curvatures =
    vtkSmartPointer < vtkCurvatures >::New();
  curvatures->SetInputConnection(smoothPolyDataFilter->GetOutputPort());
  curvatures->Update();
  vtkPolyData * curvaturePolyData = curvatures->GetOutput();
  if (curvaturePolyData == NULL)
    {
    return score;
    }
  vtkDataArray * gaussCurvature =
    curvaturePolyData->GetPointData()->GetArray("Gauss_Curvature");
  if (gaussCurvature == NULL)
    {
    return score;
    }
  vtkIdType numberOfPoints = gaussCurvature->GetNumberOfTuples();
  score = 0;
  for (vtkIdType i = 0; i < numberOfPoints; ++i)
    {
    score += std::pow(gaussCurvature->GetComponent(i,0), 2);
    }

  return score;
}


int vtkMaximizeSquaredGaussianCurvatureProjectionFilter::RequestData(
    vtkInformation *vtkNotUsed(request),
    vtkInformationVector **inputVector,
    vtkInformationVector *outputVector)
{
  // get the input and ouptut
  vtkTable *input = vtkTable::SafeDownCast(
      inputVector[0]->GetInformationObject(0)->Get(
          vtkDataObject::DATA_OBJECT()));

  vtkPolyData *output0 = vtkPolyData::SafeDownCast(
    outputVector->GetInformationObject(0)->Get(
        vtkDataObject::DATA_OBJECT()));

  vtkPolyData *output1 = vtkPolyData::SafeDownCast(
    outputVector->GetInformationObject(1)->Get(
        vtkDataObject::DATA_OBJECT()));

  vtkDataArray * inScalars= this->GetInputArrayToProcess(0,inputVector);
  if ( inScalars == NULL )
    {
    vtkErrorMacro(<<"No scalar data to process");
    return 0;
    }
  const char * inputScalarName = inScalars->GetName();

  vtkSmartPointer < vtkTable > table =
    vtkSmartPointer < vtkTable >::New();
  table->ShallowCopy(input);

  int numberColumns = table->GetNumberOfColumns();

  int bestXYZ[3] = {-1, -1, -1};
  double bestScore = -std::numeric_limits< double >::infinity();

  for (int X = 0; X < numberColumns; ++X)
    {
    // Skip if column isn't active
    if (this->GetPointArrayStatus(this->GetPointArrayName(X)) == 0) continue;

    for (int Y = (X + 1); Y < numberColumns; ++Y)
      {
      // Skip if column isn't active
      if (this->GetPointArrayStatus(this->GetPointArrayName(Y)) == 0) continue;

      for (int Z = (Y + 1); Z < numberColumns; ++Z)
        {
        // Skip if column isn't active
        if (this->GetPointArrayStatus(this->GetPointArrayName(Z)) == 0) continue;

        double score = getScore(
          inputScalarName, table, X, Y, Z, this->MaximumEdgeLength,
          this->RetainLowestValues, this->Percentile);
        if (score > bestScore)
          {
          bestScore = score;
          bestXYZ[0] = X;
          bestXYZ[1] = Y;
          bestXYZ[2] = Z;
          }
        }
      }
    }
  if (bestXYZ[1] == -1)
    {
    vtkErrorMacro( << "failed to find a projection" );
    return 0;
    }

  vtkStdString xColumnString( table->GetColumn(bestXYZ[0])->GetName() );
  vtkStdString yColumnString( table->GetColumn(bestXYZ[1])->GetName() );
  vtkStdString zColumnString( table->GetColumn(bestXYZ[2])->GetName() );

  vtkDebugMacro(
    << "The best projection is "
    << bestXYZ[0] << " (" << xColumnString << ") "
    << bestXYZ[1] << " (" << yColumnString << ") "
    << bestXYZ[2] << " (" << zColumnString << ")\n");

  vtkSmartPointer < vtkTableToPolyData > tableToPoints =
    vtkSmartPointer < vtkTableToPolyData >::New();
  tableToPoints->SetXColumnIndex(bestXYZ[0]);
  tableToPoints->SetYColumnIndex(bestXYZ[1]);
  tableToPoints->SetZColumnIndex(bestXYZ[2]);
  tableToPoints->PreserveCoordinateColumnsAsDataArraysOn();
  tableToPoints->SetInputData(table);

  vtkSmartPointer< vtkRescalePointsFilter > rescaleFilter =
    vtkSmartPointer< vtkRescalePointsFilter >::New();
  rescaleFilter->SetInputConnection( tableToPoints->GetOutputPort() );

  rescaleFilter->Update();
  output0->ShallowCopy(rescaleFilter->GetOutput());

  vtkSmartPointer < vtkPercentileSurfaceFilter > percentileSurfaceFilter =
    vtkSmartPointer < vtkPercentileSurfaceFilter >::New();
  percentileSurfaceFilter->SetInputConnection(
    rescaleFilter->GetOutputPort());
  percentileSurfaceFilter->SetPercentile(this->Percentile);
  percentileSurfaceFilter->SetRetainLowestValues(this->RetainLowestValues);
  percentileSurfaceFilter->SetMaximumEdgeLength(this->MaximumEdgeLength);
  percentileSurfaceFilter->
    SetInputArrayToProcess( 0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS,
                            inputScalarName );
  percentileSurfaceFilter->Update();
  output1->ShallowCopy(percentileSurfaceFilter->GetOutput());

  // Record the projection in the field data of the output
  vtkSmartPointer< vtkStringArray > bestDimensionNamesArray =
    vtkSmartPointer< vtkStringArray >::New();
  bestDimensionNamesArray->SetName( "Best dimensions" );
  bestDimensionNamesArray->SetNumberOfValues( 3 );
  bestDimensionNamesArray->SetValue( 0, xColumnString );
  bestDimensionNamesArray->SetValue( 1, yColumnString );
  bestDimensionNamesArray->SetValue( 2, zColumnString );

  vtkSmartPointer< vtkFieldData > fieldData =
    vtkSmartPointer< vtkFieldData >::New();
  fieldData->AddArray( bestDimensionNamesArray );

  output0->SetFieldData( fieldData );
  output1->SetFieldData( fieldData );

  return 1;
}

int
vtkMaximizeSquaredGaussianCurvatureProjectionFilter
::GetNumberOfPointArrays()
{
  int numberOfPointArrays = 0;

  vtkTable * input = vtkTable::SafeDownCast(this->GetInput());
  if (input)
    {
    numberOfPointArrays = input->GetNumberOfColumns();
    }

  return numberOfPointArrays;
}

const char*
vtkMaximizeSquaredGaussianCurvatureProjectionFilter
::GetPointArrayName(int index)
{
  const char *name = "<unnamed>";

  vtkTable * input = vtkTable::SafeDownCast(this->GetInput());
  if (input)
    {
    name = input->GetColumn(index)->GetName();
    }

  return name;
}

void
vtkMaximizeSquaredGaussianCurvatureProjectionFilter
::SetPointArrayStatus(const char *name, int status)
{
  std::string nameString(name);
  this->Internal->ActivePointArrays[nameString] = status;

  this->Modified();
}

int
vtkMaximizeSquaredGaussianCurvatureProjectionFilter
::GetPointArrayStatus(const char *name)
{
  // Check if the map has an entry. If not, create one and set it to be
  // enabled. Otherwise, return the status.
  std::string nameString(name);

  if (this->Internal->ActivePointArrays.count(nameString) == 0)
    {
    this->Internal->ActivePointArrays[nameString] = 1;
    }

  return this->Internal->ActivePointArrays[nameString];
}
