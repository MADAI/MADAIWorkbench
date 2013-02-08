#include "vtkVectorComparisonGlyphFilter.h"

#include "vtkAlgorithm.h"
#include "vtkCellArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMath.h"
#include "vtkPointData.h"
#include "vtkStreamingDemandDrivenPipeline.h"


vtkStandardNewMacro(vtkVectorComparisonGlyphFilter);


//----------------------------------------------------------------------------
vtkVectorComparisonGlyphFilter::vtkVectorComparisonGlyphFilter()
{
  this->ScaleFactor = 1.0;
  this->DiskResolution = 8;

  this->SetNumberOfInputPorts(2);
  this->SetNumberOfOutputPorts(1);
}

//----------------------------------------------------------------------------
vtkVectorComparisonGlyphFilter::~vtkVectorComparisonGlyphFilter()
{

}

//----------------------------------------------------------------------------
void vtkVectorComparisonGlyphFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
int vtkVectorComparisonGlyphFilter::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // Get the information objects
  vtkInformation *inInfo0 = inputVector[0]->GetInformationObject(0);
  vtkInformation *inInfo1 = inputVector[1]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // Get the input and output
  vtkDataSet *input0 = vtkDataSet::SafeDownCast(
    inInfo0->Get(vtkDataObject::DATA_OBJECT()));
  vtkDataSet *input1 = vtkDataSet::SafeDownCast(
    inInfo1->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  int numInput0Points = input0->GetNumberOfPoints();

  vtkPointData * pd0 = input0->GetPointData();
  vtkDataArray * vectors0 = pd0->GetVectors();

  vtkPointData * pd1 = input1->GetPointData();
  vtkDataArray * vectors1 = pd1->GetVectors();

  if (!vectors0)
    {
    vtkErrorMacro( << "No vectors available in input 0!");
    return 0;
    }

  if (!vectors1)
    {
    vtkErrorMacro( << "No vectors available in input 1!");
    return 0;
    }

  vtkPoints * newPoints = vtkPoints::New();
  newPoints->Allocate(numInput0Points);

  vtkCellArray * polys = vtkCellArray::New();
  polys->Allocate(numInput0Points);

  for ( vtkIdType id = 0; id < numInput0Points; ++id )
    {
    double pt[3];
    input0->GetPoint(id, pt);

    double v0[3], v1[3];
    vectors0->GetTuple(id, v0);
    vectors1->GetTuple(id, v1);

    // Scale vectors and compute the tips of the vectors
    double t0[3], t1[3];
    for ( int i = 0; i < 3; ++i )
      {
      v0[i] *= this->ScaleFactor;
      v1[i] *= this->ScaleFactor;
      t0[i] = pt[i] + v0[i];
      t1[i] = pt[i] + v1[i];
      }

    // Get the radii of the two vector points
    double r0 = vtkMath::Norm( v0 );
    double r1 = vtkMath::Norm( v1 );

    // Now normalize the vectors
    vtkMath::Normalize( v0 );
    vtkMath::Normalize( v1 );

    double tip0[3], tip1[3];
    for ( int i = 0; i < 3; ++i )
      {
      tip0[i] = r0 * v0[i];
      tip1[i] = r1 * v1[i];
      }

    newPoints->InsertNextPoint(pt);
    newPoints->InsertNextPoint(tip0);
    newPoints->InsertNextPoint(tip1);

    polys->InsertNextCell(1, &id );
    }

  output->SetPoints(newPoints);
  newPoints->Delete();
  //output->SetPolys(polys);
  polys->Delete();

  return 1;
}

//----------------------------------------------------------------------------
int vtkVectorComparisonGlyphFilter::RequestUpdateExtent(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // I'm not sure what all this does. It's copied from vtkGlyph3D::RequestUpdateExtent

  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *sourceInfo = inputVector[1]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  if (sourceInfo)
    {
    sourceInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER(),
                    0);
    sourceInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_PIECES(),
                    1);
    sourceInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_GHOST_LEVELS(),
                    0);
    }
  inInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER(),
              outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER()));
  inInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_PIECES(),
              outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_PIECES()));
  inInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_GHOST_LEVELS(),
              outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_GHOST_LEVELS()));
  inInfo->Set(vtkStreamingDemandDrivenPipeline::EXACT_EXTENT(), 1);

  return 1;
}

//----------------------------------------------------------------------------
int vtkVectorComparisonGlyphFilter::FillInputPortInformation(int port, vtkInformation *info)
{
  if (port == 0 || port == 1)
    {
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
    info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 0);
    return 1;
    }

  return 0;
}
