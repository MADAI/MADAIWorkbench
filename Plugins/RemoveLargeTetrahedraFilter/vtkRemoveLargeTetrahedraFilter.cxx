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

#include "vtkCellArray.h"
#include "vtkDataObject.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkUnsignedCharArray.h"
#include "vtkUnstructuredGrid.h"


#include "vtkRemoveLargeTetrahedraFilter.h"
vtkStandardNewMacro(vtkRemoveLargeTetrahedraFilter)

vtkRemoveLargeTetrahedraFilter::vtkRemoveLargeTetrahedraFilter()
{
  this->SetNumberOfInputPorts(1);
  this->SetNumberOfOutputPorts(1);
  this->MaximumEdgeLength = 0.25;
}

vtkRemoveLargeTetrahedraFilter::~vtkRemoveLargeTetrahedraFilter()
{
}


int vtkRemoveLargeTetrahedraFilter::RequestData(
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
  vtkUnstructuredGrid *output = vtkUnstructuredGrid::SafeDownCast(
      outInfo->Get(vtkDataObject::DATA_OBJECT()));

  double dist2 = this->MaximumEdgeLength*this->MaximumEdgeLength;

  vtkSmartPointer< vtkCellArray > newCellArray =
    vtkSmartPointer< vtkCellArray >::New();

  vtkIdType numberOfPoints = input->GetNumberOfPoints();
  vtkIdType numberOfCells = input->GetNumberOfCells();

  std::vector< bool > pointsFound(numberOfPoints, false);

  for(vtkIdType cellId = 0; cellId < numberOfCells; ++cellId)
    {
    int cellType = input->GetCellType(cellId);
    if (cellType == VTK_TETRA)
      {
      vtkCell * cell = input->GetCell(cellId);
      int cellType = cell->GetCellType();
      vtkIdType numberOfCellPoints = cell->GetNumberOfPoints();
      if (numberOfCellPoints == 4)
        {
        vtkIdList * idList = cell->GetPointIds();
        vtkIdType thisCellPtIds[4];
        double xyz[4][3];
        for (vtkIdType i = 0; i < 4; ++i)
          {
          thisCellPtIds[i] = idList->GetId(i);
          input->GetPoint(thisCellPtIds[i], xyz[i]);
          }
        if ((vtkMath::Distance2BetweenPoints(xyz[0], xyz[1]) < dist2) &&
            (vtkMath::Distance2BetweenPoints(xyz[0], xyz[2]) < dist2) &&
            (vtkMath::Distance2BetweenPoints(xyz[0], xyz[3]) < dist2) &&
            (vtkMath::Distance2BetweenPoints(xyz[1], xyz[2]) < dist2) &&
            (vtkMath::Distance2BetweenPoints(xyz[1], xyz[3]) < dist2) &&
            (vtkMath::Distance2BetweenPoints(xyz[2], xyz[3]) < dist2))
          {
          newCellArray->InsertNextCell(4);
          for(int j = 0; j < 4; ++j)
            {
            newCellArray->InsertCellPoint(thisCellPtIds[j]);
            pointsFound[thisCellPtIds[j]] = true;
            }
          }
        }
      }
    }
  vtkIdType numberOfTetrahedrons = newCellArray->GetNumberOfCells();
  for (vtkIdType i = 0; i < numberOfPoints; i++)
    {
    if (! pointsFound[i])
      {
      newCellArray->InsertNextCell(1);
      newCellArray->InsertCellPoint(i);
      }
    }
  vtkIdType ncells = newCellArray->GetNumberOfCells();
  vtkSmartPointer < vtkIdTypeArray > cellLocations =
    vtkSmartPointer < vtkIdTypeArray >::New();
  cellLocations->Allocate(ncells);
  vtkSmartPointer < vtkUnsignedCharArray > newCellTypes =
    vtkSmartPointer < vtkUnsignedCharArray >::New();
  newCellTypes->Allocate(ncells);

  vtkIdType cellOffset = 0;
  for (vtkIdType i = 0; i < numberOfTetrahedrons; i++)
    {
    newCellTypes->InsertNextValue(VTK_TETRA);
    cellLocations->InsertNextValue(cellOffset);
    cellOffset += 5;
    }
  for (vtkIdType i = numberOfTetrahedrons; i < ncells; i++)
    {
    newCellTypes->InsertNextValue(VTK_VERTEX);
    cellLocations->InsertNextValue(cellOffset);
    cellOffset += 2;
    }
  output->ShallowCopy(input);
  output->SetCells(newCellTypes, cellLocations, newCellArray, NULL, NULL);

  return 1;
}


//----------------------------------------------------------------------------
void vtkRemoveLargeTetrahedraFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "MaximumEdgeLength: " << this->MaximumEdgeLength << "\n";
}
