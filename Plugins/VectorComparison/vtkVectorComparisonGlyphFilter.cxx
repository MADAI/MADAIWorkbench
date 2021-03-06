#include "vtkVectorComparisonGlyphFilter.h"

#include "vtkAlgorithm.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkLine.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolygon.h"
#include "vtkShortArray.h"
#include "vtkSmartPointer.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkTransform.h"


vtkStandardNewMacro(vtkVectorComparisonGlyphFilter);


//----------------------------------------------------------------------------
vtkVectorComparisonGlyphFilter::vtkVectorComparisonGlyphFilter()
{
  this->ScaleFactor = 1.0;
  this->DiskResolution = 8;
  this->MagnitudeDifferenceAlignmentMode = VTK_HALF_VECTOR;

  this->SetNumberOfInputPorts( 2 );
  this->SetNumberOfOutputPorts( 2 );

  this->SetInputArrayToProcess(0,0,0,vtkDataObject::FIELD_ASSOCIATION_POINTS,
                               vtkDataSetAttributes::VECTORS);
  this->SetInputArrayToProcess(1,1,0,vtkDataObject::FIELD_ASSOCIATION_POINTS,
                               vtkDataSetAttributes::VECTORS);
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
  vtkInformation *inInfo0 = inputVector[0]->GetInformationObject( 0 );
  vtkInformation *inInfo1 = inputVector[1]->GetInformationObject( 0 );
  vtkInformation *diskInfo      = outputVector->GetInformationObject( 0 );
  vtkInformation *magnitudeInfo = outputVector->GetInformationObject( 1 );

  // Get the input and output
  vtkDataSet *input0 = vtkDataSet::SafeDownCast(
    inInfo0->Get(vtkDataObject::DATA_OBJECT()));
  vtkDataSet *input1 = vtkDataSet::SafeDownCast(
    inInfo1->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData *diskOutput = vtkPolyData::SafeDownCast(
    diskInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData *magnitudeOutput = vtkPolyData::SafeDownCast(
    magnitudeInfo->Get(vtkDataObject::DATA_OBJECT()));

  int numInput0Points = input0->GetNumberOfPoints();

  vtkDataArray * vectors0 = this->GetInputArrayToProcess( 0, inputVector );
  vtkDataArray * vectors1 = this->GetInputArrayToProcess( 1, inputVector);

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

  /////////////////////////////////
  // Disk glyphs
  /////////////////////////////////

  diskOutput->Allocate( numInput0Points );
  vtkSmartPointer< vtkPoints > diskPoints =
    vtkSmartPointer< vtkPoints >::New();
  diskPoints->Allocate( numInput0Points );
  diskOutput->SetPoints( diskPoints );

  magnitudeOutput->Allocate( numInput0Points );
  vtkSmartPointer< vtkPoints > magnitudePoints =
    vtkSmartPointer< vtkPoints >::New();
  magnitudePoints->Allocate( 2 * numInput0Points );
  magnitudeOutput->SetPoints( magnitudePoints );

  vtkSmartPointer< vtkPolygon > poly =
    vtkSmartPointer< vtkPolygon >::New();
  vtkSmartPointer< vtkTransform > tform =
    vtkSmartPointer< vtkTransform >::New();
  vtkSmartPointer< vtkLine > line =
    vtkSmartPointer< vtkLine >::New();

  // Add array encoding which vector field is largest
  vtkSmartPointer< vtkShortArray > largestVectorArray =
    vtkSmartPointer< vtkShortArray >::New();
  largestVectorArray->SetName( "Larger Vector" );
  largestVectorArray->SetNumberOfValues( numInput0Points );

  magnitudeOutput->GetCellData()->AddArray( largestVectorArray );

  for ( vtkIdType id = 0; id < numInput0Points; ++id )
    {
    double pt[3];
    input0->GetPoint( id, pt );

    double v0[3], v1[3];
    vectors0->GetTuple( id, v0 );
    vectors1->GetTuple( id, v1 );

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

    // Pick an up vector
    double up[3] = {0.0, 0.0, 0.0};
    if ( fabs( vtkMath::Dot( v0, v1) ) < 1-1.0e-5)
      {
      vtkMath::Cross( v0, v1, up );
      }
    else
      {
      double dummy[3];
      vtkMath::Perpendiculars( v0, up, dummy, 0 );
      }

    double right[3] = { v1[0], v1[1], v1[2] };
    double forward[3] = { 0.0, 0.0, 0.0 };
    vtkMath::Cross( right, up, forward );

    double maxTheta = acos( vtkMath::Dot( v0, v1 ) );

    poly->GetPointIds()->Reset();
    poly->GetPointIds()->SetNumberOfIds( this->DiskResolution );

    double r = ( r0 > r1 ) ? r0 : r1;

    int pointsPerArc = this->DiskResolution - 1;
    for ( int j = 0; j < pointsPerArc; ++j )
      {
      double t = static_cast< double >( j ) /
        static_cast< double >( pointsPerArc - 1 );

      double theta = t * maxTheta;

      // Now rotate b by theta
      tform->Identity();
      tform->RotateWXYZ( vtkMath::DegreesFromRadians( -theta ), up );

      double rot[3] = {0.0, 0.0, 0.0};
      tform->TransformPoint( v1, rot );

      double diskPoint[3];
      for ( int i = 0; i < 3; ++i )
        {
        diskPoint[i] = pt[i] + r * rot[i];
        }

      diskPoints->InsertNextPoint( diskPoint );
      poly->GetPointIds()->SetId( j, this->DiskResolution * id + j );
      }

    diskPoints->InsertNextPoint( pt );
    poly->GetPointIds()->SetId( pointsPerArc, this->DiskResolution * id + pointsPerArc );

    diskOutput->InsertNextCell( poly->GetCellType(), poly->GetPointIds() );

    /////////////////////////////////
    // Magnitude difference glyphs //
    /////////////////////////////////

    largestVectorArray->SetValue( id, r0 > r1 ? 0 : 1 );

    double minR = std::min( r0, r1 );
    double maxR = std::max( r0, r1 );

    double p0[3], p1[3];
    switch ( this->MagnitudeDifferenceAlignmentMode )
      {
      case VTK_FIRST_VECTOR:
      {
      for ( int i = 0; i < 3; ++i )
        {
        p0[i] = pt[i] + minR*v0[i];
        p1[i] = pt[i] + maxR*v0[i];
        }
      }
      break;

      case VTK_SECOND_VECTOR:
      {
      for ( int i = 0; i < 3; ++i )
        {
        p0[i] = pt[i] + minR*v1[i];
        p1[i] = pt[i] + maxR*v1[i];
        }
      }
      break;

      case VTK_SMALLEST_VECTOR:
      {
      for ( int i = 0; i < 3; ++i )
        {
        double smallestV = v0[i];
        if ( r0 > r1 )
          {
          smallestV = v1[i];
          }
        p0[i] = pt[i] + minR*smallestV;
        p1[i] = pt[i] + maxR*smallestV;
        }
      }
      break;

      case VTK_LARGEST_VECTOR:
      {
      for ( int i = 0; i < 3; ++i )
        {
        double largestV = v0[i];
        if ( r1 > r0 )
          {
          largestV = v1[i];
          }
        p0[i] = pt[i] + minR*largestV;
        p1[i] = pt[i] + maxR*largestV;
        }
      }
      break;

      case VTK_HALF_VECTOR:
      default:
      {
      // Compute half angle vector between vectors
      double sumSquared = 0.0;
      for ( int i = 0; i < 3; ++i )
        {
        sumSquared += (v0[i] + v1[i]) * (v0[i] + v1[i]);
        }
      double magnitudeSumV0V1 = sqrt( sumSquared );

      for ( int i = 0; i < 3; ++i )
        {
        double h = (v0[i] + v1[i]) / magnitudeSumV0V1;

        // First point is displaced by vector 0 magnitude along the
        // half vector, which is already scaled by ScaleFactor
        p0[i] = pt[i] + r0*h;

        // Second point is displaced by vector 1 magnitude along the
        // half vector, which is already scaled by ScaleFactor
        p1[i] = pt[i] + r1*h;
        }
      }
      break;

      }
    vtkIdType pid0 = magnitudePoints->InsertNextPoint( p0 );
    vtkIdType pid1 = magnitudePoints->InsertNextPoint( p1 );

    // Create a vtkLine to add to the output
    line->GetPointIds()->Reset();
    line->GetPointIds()->SetNumberOfIds( 2 );
    line->GetPointIds()->SetId( 0, pid0 );
    line->GetPointIds()->SetId( 1, pid1 );

    magnitudeOutput->InsertNextCell( line->GetCellType(),
				     line->GetPointIds() );
    }

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

  if (sourceInfo)
    {
    sourceInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER(),
                    0);
    sourceInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_PIECES(),
                    1);
    sourceInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_GHOST_LEVELS(),
                    0);
    }

  for ( int i = 0; i < this->GetNumberOfOutputPorts(); ++i )
    {
      vtkInformation *outInfo = outputVector->GetInformationObject( i );
      inInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER(),
		  outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER()));
      inInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_PIECES(),
		  outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_PIECES()));
      inInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_GHOST_LEVELS(),
		  outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_GHOST_LEVELS()));
      inInfo->Set(vtkStreamingDemandDrivenPipeline::EXACT_EXTENT(), 1);
    }

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
