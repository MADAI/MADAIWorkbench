/*=========================================================================
  Program:   Visualization Toolkit - Gaussian Scalar Splat
  Module:    vtkGaussianScalarSplatter.cxx

  See README.txt for more information.

  Written 2011 Hal Canary <hal@cs.unc.edu> for the MADAI project
  <http://MADAI.us>.  Based on vtkGaussianSplatter, Copyright (c) Ken
  Martin, Will Schroeder, Bill Lorensen.  All rights reserved.

  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

    This software is distributed WITHOUT ANY WARRANTY; without even
    the implied warranty of MERCHANTABILITY or FITNESS FOR A
    PARTICULAR PURPOSE.  See the above copyright notice for more
    information.

=========================================================================*/
#include "vtkGaussianScalarSplatter.h"

#include "vtkDoubleArray.h"
#include "vtkExtentTranslator.h"
#include "vtkIdList.h"
#include "vtkImageData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMultiThreader.h"
#include "vtkObjectFactory.h"
#include "vtkOnePieceExtentTranslator.h"
#include "vtkPointData.h"
#include "vtkPointLocator.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkRectilinearGrid.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkStructuredGrid.h"
#include "vtkStructuredPoints.h"
#include "vtkUnstructuredGrid.h"

// This enables use of a vtkPointLocator to find only the points that
// affect a pixel. The downside of enabling this is option is that the
// input data needs to be DeepCopy'ed once per thread. The downside of
// not enabling it is that all points in the input will be considered
// for splatting, even if they are many standard deviations away
//#define USE_LOCATOR

const double NUMBER_OF_STD_DEVIATIONS = 4.0;
const char DENSITY_STRING [] = "_Density";
const double vtkGaussianScalarSplatter::DEFAULT_STANDARD_DEVIATION = 1.0;

// This is public domain code from John D. Cook based on
// the formula 7.1.26 in the Handbook of Mathematical Functions
// by Abramowitz and Stegun. http://www.johndcook.com/cpp_erf.html
// It really should be put somewhere better than here, but this will
// work for now.
double vtkGaussianScalarSplatter_erf(double x)
{
  // constants
  double a1 =  0.254829592;
  double a2 = -0.284496736;
  double a3 =  1.421413741;
  double a4 = -1.453152027;
  double a5 =  1.061405429;
  double p  =  0.3275911;

  // Save the sign of x
  int sign = 1;
  if (x < 0)
    sign = -1;
  x = fabs(x);

  // A&S formula 7.1.26
  double t = 1.0/(1.0 + p*x);
  double y = 1.0 - (((((a5*t + a4)*t) + a3)*t + a2)*t + a1)*t*exp(-x*x);

  return sign*y;
}

class SliceDataType {
public:
  vtkDataSet ** Input;
  vtkDoubleArray * NumberDensity;
  std::vector<vtkDataArray *> * OutputDataArrays;
  std::vector<vtkDataArray *> * InputDataArrays;
  double Sqrt2sigma;
  double Radius;
  double Origin[3];
  double Spacing[3];
  int SampleDimensions[3];
};
static void vtkGaussianScalarSplatter_ProcessSlice(SliceDataType * sliceData, int sliceIndex, int threadId);
static VTK_THREAD_RETURN_TYPE vtkGaussianScalarSplatter_ThreadedExecute( void * arg );

vtkStandardNewMacro(vtkGaussianScalarSplatter);

// Construct object with dimensions=(50,50,50); automatic computation of
// bounds;
vtkGaussianScalarSplatter::vtkGaussianScalarSplatter()
{
  this->SampleDimensions[0] = 50;
  this->SampleDimensions[1] = 50;
  this->SampleDimensions[2] = 50;

  this->StandardDeviation = DEFAULT_STANDARD_DEVIATION;

  this->ModelBounds[0] = 0.0;
  this->ModelBounds[1] = 0.0;
  this->ModelBounds[2] = 0.0;
  this->ModelBounds[3] = 0.0;
  this->ModelBounds[4] = 0.0;
  this->ModelBounds[5] = 0.0;
}

//----------------------------------------------------------------------------
int vtkGaussianScalarSplatter::RequestUpdateExtent(vtkInformation* vtkNotUsed(request),
                                                   vtkInformationVector** inputVector,
                                                   vtkInformationVector* outputVector)
{
  // get the info objects
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);

  int numPieces = 1;
  int piece = 0;
  int ghostLevel = 0;
  // Use the output piece request to break up the input.
  // If not specified, use defaults.
  if (outInfo->Has(
        vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_PIECES()))
    {
    numPieces = outInfo->Get(
      vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_PIECES());
    }
  if (outInfo->Has(
        vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER()))
    {
    piece = outInfo->Get(
      vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER());
    }
  if (outInfo->Has(
        vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_GHOST_LEVELS()))
    {
    ghostLevel = outInfo->Get(
      vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_GHOST_LEVELS());
    }
  vtkDataObject* data = inInfo->Get(vtkDataObject::DATA_OBJECT());
  // If input extent is piece based, just pass the update requests
  // from the output. Even though the output extent is structured,
  // piece-based request still gets propagated. This will not work
  // if there was no piece based request to start with. That is handled
  // above.
  if(data->GetExtentType() == VTK_PIECES_EXTENT)
    {
    inInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_PIECES(),
                numPieces);
    inInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER(),
                piece);
    inInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_GHOST_LEVELS(),
                ghostLevel);
    }
  else if(data->GetExtentType() == VTK_3D_EXTENT)
    {
    int* inWholeExtent =
      inInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT());

    vtkExtentTranslator* translator =
      vtkExtentTranslator::SafeDownCast(
        inInfo->Get(vtkStreamingDemandDrivenPipeline::EXTENT_TRANSLATOR()));
    if(translator)
      {
      translator->SetWholeExtent(inWholeExtent);
      translator->SetPiece(piece);
      translator->SetNumberOfPieces(numPieces);
      translator->SetGhostLevel(ghostLevel);
      translator->PieceToExtent();
      inInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(),
                  translator->GetExtent(),
                  6);
      }
    else
      {
      inInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(),
                  inWholeExtent,
                  6);
      }
    }


  return 1;
}

//----------------------------------------------------------------------------
int vtkGaussianScalarSplatter::RequestInformation (
  vtkInformation * vtkNotUsed(request),
  vtkInformationVector ** vtkNotUsed( inputVector ),
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  // use model bounds if set
  this->Origin[0] = 0;
  this->Origin[1] = 0;
  this->Origin[2] = 0;
  if ( this->ModelBounds[0] < this->ModelBounds[1] &&
       this->ModelBounds[2] < this->ModelBounds[3] &&
       this->ModelBounds[4] < this->ModelBounds[5] )
    {
    this->Origin[0] = this->ModelBounds[0];
    this->Origin[1] = this->ModelBounds[2];
    this->Origin[2] = this->ModelBounds[4];
    }

  outInfo->Set(vtkDataObject::ORIGIN(), this->Origin, 3);

  int i;
  for (i=0; i<3; i++)
    {
    if (this->SampleDimensions[i] > 1)
      {
      this->Spacing[i] = (this->ModelBounds[2*i+1] - this->ModelBounds[2*i])
        / (this->SampleDimensions[i] - 1);
      }
    else
      {
      this->Spacing[i] = 1.0;
      }
    if ( this->Spacing[i] <= 0.0 )
      {
      this->Spacing[i] = 1.0;
      }
    }
  outInfo->Set(vtkDataObject::SPACING(),this->Spacing,3);

  outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(),
               0, this->SampleDimensions[0] - 1,
               0, this->SampleDimensions[1] - 1,
               0, this->SampleDimensions[2] - 1);
  vtkDataObject::SetPointDataActiveScalarInfo(outInfo, VTK_DOUBLE, 1);

  // Copied from vtkFastSplatter::RequestInformation
  // Setup ExtentTranslator so that all downstream piece requests are
  // converted to whole extent update requests, as need by this filter.
  vtkStreamingDemandDrivenPipeline* sddp =
    vtkStreamingDemandDrivenPipeline::SafeDownCast(this->GetExecutive());
  if (strcmp(
      sddp->GetExtentTranslator(outInfo)->GetClassName(),
      "vtkOnePieceExtentTranslator") != 0)
    {
    vtkExtentTranslator* et = vtkOnePieceExtentTranslator::New();
    sddp->SetExtentTranslator(outInfo, et);
    et->Delete();
    }

  return 1;
}

//----------------------------------------------------------------------------
int vtkGaussianScalarSplatter::RequestData(
  vtkInformation* vtkNotUsed( request ),
  vtkInformationVector** inputVector,
  vtkInformationVector* outputVector)
{

  // get the data object
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  vtkImageData *output = vtkImageData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  output->SetExtent(
    outInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT()));

  vtkIdType numPts, idx;
  int i;

  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkDataSet *input = vtkDataSet::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));

  int numNewPts = (this->SampleDimensions[0] *
		   this->SampleDimensions[1] *
		   this->SampleDimensions[2]);

  // loop through the input data arrays to make output
  int numberOfInputDataArrays = input->GetPointData()->GetNumberOfArrays();
  int inputDataArraysIndex;
  int dataType;			// VTK_FLOAT or VTK_DOUBLE
  vtkDataArray * iDataArray;    // the ith input data array
  vtkDataArray * oDataArray;    // output data array
  int numberOfComponents;
  int compIdx;          // component index (for vectors)
  char * oldname, * newname ;
  int abortExecute;
  vtkIdType progressInterval;

  std::vector<vtkDataArray *> outputDataArrays; // pointers to the arrays I'm going to splat.
  std::vector<vtkDataArray *> inputDataArrays; // pointers to the arrays I'm splatting into

  // sliceData is a chunk of memory containing information I'm going
  // to pass to the vtkGaussianScalarSplatter_ProcessSlice() function.
  SliceDataType sliceData;

  // Initialize multiple threads
  vtkMultiThreader * threader = vtkMultiThreader::New();
  int numberOfThreads = vtkMultiThreader::GetGlobalDefaultNumberOfThreads();
  vtkMultiThreader::SetGlobalMaximumNumberOfThreads(numberOfThreads);
  if (numberOfThreads > this->SampleDimensions[2])
    {
    numberOfThreads = this->SampleDimensions[2];
    }
  threader->SetNumberOfThreads(numberOfThreads);

  sliceData.Input = new vtkDataSet*[numberOfThreads];
  for (int threadId = 0; threadId < numberOfThreads; threadId++)
    {
    switch (input->GetDataObjectType())
      {
      case VTK_POLY_DATA:
        sliceData.Input[threadId] = vtkPolyData::New();
        break;

      case VTK_IMAGE_DATA:
        sliceData.Input[threadId] = vtkImageData::New();
        break;

      case VTK_STRUCTURED_POINTS:
        sliceData.Input[threadId] = vtkStructuredPoints::New();
        break;

      case VTK_STRUCTURED_GRID:
        sliceData.Input[threadId] = vtkStructuredGrid::New();
        break;

      case VTK_UNSTRUCTURED_GRID:
        sliceData.Input[threadId] = vtkUnstructuredGrid::New();
        break;

      case VTK_RECTILINEAR_GRID:
        sliceData.Input[threadId] = vtkRectilinearGrid::New();
        break;

      default:
        vtkErrorMacro(<< "Unsupported data set type " << input->GetClassName());
        break;
      }
#ifdef USE_LOCATOR
    sliceData.Input[threadId]->DeepCopy(input);
#else
    sliceData.Input[threadId]->ShallowCopy(input);
#endif
    }
  sliceData.OutputDataArrays = (& outputDataArrays);
  sliceData.InputDataArrays = (& inputDataArrays);

  //  Make sure points are available
  if ( (numPts = input->GetNumberOfPoints()) < 1 )
    {
    vtkDebugMacro(<<"No points to splat!");
    return 1;
    }
  output->SetDimensions(this->GetSampleDimensions());
  this->ComputeModelBounds(input, output, outInfo);
  // NOW this->Spacing AND this->Origin IS SET.

  double largest_dim = 0.0;
  for (i = 0; i < 3; i++)
    {
    sliceData.Origin[i] = this->Origin[i];
    sliceData.Spacing[i] = this->Spacing[i];
    sliceData.SampleDimensions[i] = this->SampleDimensions[i];

    if (this->Spacing[i] > largest_dim)
      {
      largest_dim = this->Spacing[i];
      }
    }
  sliceData.Sqrt2sigma = (sqrt(2.0) * this->StandardDeviation);
  sliceData.Radius = ((this->StandardDeviation * NUMBER_OF_STD_DEVIATIONS)
                      + (largest_dim / 2.0));

  // Inserting a new array.
  sliceData.NumberDensity = vtkDoubleArray::New();
  sliceData.NumberDensity->SetNumberOfComponents(1);
  sliceData.NumberDensity->SetName("NumberDensity");
  // PLEASE NOTE: Summing up all the Number Density and multiplying
  // by the voxel volume should return the original number of points.
  sliceData.NumberDensity->SetNumberOfTuples(numNewPts);
  output->GetPointData()->AddArray(sliceData.NumberDensity);
  for (idx=0; idx<numNewPts; idx++)
    {
    sliceData.NumberDensity->SetComponent(idx, 0, 0.0); //initialize to ZERO
    }
  //create output arrays after input data arrays
  for (inputDataArraysIndex = 0;
       inputDataArraysIndex < numberOfInputDataArrays;
       inputDataArraysIndex++)
    {
    iDataArray = input->GetPointData()->GetArray(inputDataArraysIndex);
    dataType = iDataArray->GetDataType();
    if ((dataType != VTK_DOUBLE) && (dataType != VTK_FLOAT))
      {
      continue;
      // I'm not sure what other types I might find, so let us ignore them.
      }
    oDataArray = vtkDataArray::CreateDataArray(dataType);

    oldname = iDataArray->GetName();
    newname = new char [strlen(oldname) + sizeof(DENSITY_STRING) + 1];
    strcpy(newname, oldname);
    strcpy(newname + strlen(oldname), DENSITY_STRING);
    oDataArray->SetName(newname);
    delete[] newname;

    numberOfComponents = iDataArray->GetNumberOfComponents();
    oDataArray->SetNumberOfComponents(numberOfComponents);
    // allocate enough memory
    oDataArray->SetNumberOfTuples(numNewPts);
    output->GetPointData()->AddArray(oDataArray);
    for (idx = 0; idx < numNewPts; idx++)
      {
      for(compIdx = 0; compIdx < numberOfComponents; compIdx++)
        {
        oDataArray->SetComponent(idx, 0, 0.0); //initialize to ZERO
        }
      }
    outputDataArrays.push_back(oDataArray);
    inputDataArrays.push_back(iDataArray);
    }

  vtkDebugMacro(<< "Splatting data");
  abortExecute = 0;
  progressInterval = numPts/100 + 1;

  // Run the threads
  threader->SetSingleMethod(vtkGaussianScalarSplatter_ThreadedExecute,
			    static_cast<void *>(&sliceData));
  threader->SingleMethodExecute();
  threader->Delete();

  // Free the copies of the input
  for (int threadId = 0; threadId < numberOfThreads; threadId++)
    {
    sliceData.Input[threadId]->Delete();
    }
  delete[] sliceData.Input;

  vtkDebugMacro(<< "Splatted " << numPts << " points");
  return 1;
}

//----------------------------------------------------------------------------
static VTK_THREAD_RETURN_TYPE vtkGaussianScalarSplatter_ThreadedExecute( void * arg )
{
  vtkMultiThreader::ThreadInfo * threadInfo;
  SliceDataType * threadData;
  int sliceIndex, threadId, threadCount, numberSlices;
  threadInfo = static_cast<vtkMultiThreader::ThreadInfo *>(arg);
  threadData = static_cast<SliceDataType *>(threadInfo->UserData);
  threadId = threadInfo->ThreadID;
  threadCount = threadInfo->NumberOfThreads;
  numberSlices = threadData->SampleDimensions[2];

  for (sliceIndex = threadId;
       sliceIndex < numberSlices;
       sliceIndex += threadCount)
    {
    vtkGaussianScalarSplatter_ProcessSlice(threadData, sliceIndex, threadId);
    }
  return VTK_THREAD_RETURN_VALUE;
}


//----------------------------------------------------------------------------
// Static Function called by vtkGaussianScalarSplatter::RequestData()
// this function will modify
//   sliceData->outputDataArrays->at(*)->GetComponent(sliceData->numberDensity->GetComponent(index,0)
//    (for the range of indices corresponding to this slice)
// Other values will be left alone.
static void vtkGaussianScalarSplatter_ProcessSlice(SliceDataType * sliceData, int sliceIndex, int threadId)
{
  double voxLoc[3], voxMin[3], voxMax[3];
  int vox[2]; // vox[2] would be sliceIndex;

  vtkDataSet * input = sliceData->Input[threadId];

  vtkPointLocator * pointLocator = vtkPointLocator::New();
  pointLocator->SetDataSet(input);

  vtkDoubleArray * numberDensity = sliceData->NumberDensity;
  std::vector<vtkDataArray *> * outputDataArrays = sliceData->OutputDataArrays;
  std::vector<vtkDataArray *> * inputDataArrays = sliceData->InputDataArrays;
  double sqrt2sigma = sliceData->Sqrt2sigma;

  voxLoc[2] = sliceData->Origin[2] + (sliceData->Spacing[2] * sliceIndex);
  voxMin[2] = voxLoc[2] - (sliceData->Spacing[2] / 2.0);
  voxMax[2] = voxLoc[2] + (sliceData->Spacing[2] / 2.0);

  vtkIdList * closePoints = vtkIdList::New();
  int sliceSize = (sliceData->SampleDimensions[0] *
		   sliceData->SampleDimensions[1]);
  double pointCoords[3]; // double[3] coords of a point
  int ptId;
  int dataArrayIdx,compIdx, numberOfComponents, idx;
  vtkDataArray * oDataArray, * iDataArray;

  long int erfsCounted = 0;

  // Compute the voxel volume
  double voxelVolume = sliceData->Spacing[0] *
                       sliceData->Spacing[1] *
		       sliceData->Spacing[2];

  for (vox[1] = 0,
         voxLoc[1] = sliceData->Origin[1],
         voxMin[1] = voxLoc[1] - (sliceData->Spacing[1] / 2.0),
         voxMax[1] = voxLoc[1] + (sliceData->Spacing[1] / 2.0);
       vox[1] < sliceData->SampleDimensions[1];
       vox[1]++,
         voxMin[1] = voxMax[1],
         voxMax[1] += sliceData->Spacing[1],
         voxLoc[1] += sliceData->Spacing[1])
    {
    idx = (vox[1] * sliceData->SampleDimensions[0]) + (sliceIndex * sliceSize);
    for (vox[0] = 0,
           voxLoc[0] = sliceData->Origin[0],
           voxMin[0] = voxLoc[0] - (sliceData->Spacing[0] / 2.0),
           voxMax[0] = voxLoc[0] + (sliceData->Spacing[0] / 2.0);
         vox[0] < sliceData->SampleDimensions[0];
         vox[0]++,
           voxMin[0] = voxMax[0],
           voxMax[0] += sliceData->Spacing[0],
           voxLoc[0] += sliceData->Spacing[0],
           idx++)
      {
      closePoints->Reset();
#ifdef USE_LOCATOR
      // NOTE: if this is re-enabled in the future, we should
      // make sure to check if any of the SampleDimensions is 1.
      // If so, we need to project the data in that dimension
      // down to zero so that the locator works correctly.

      pointLocator->FindPointsWithinRadius(sliceData->radius,
                                           voxLoc, closePoints);
      erfsCounted += closePoints->GetNumberOfIds();
      for (nearPoint = 0;
           nearPoint < closePoints->GetNumberOfIds();
           nearPoint++)
        {
        ptId = closePoints->GetId(nearPoint);
#else
      erfsCounted += input->GetNumberOfPoints();
      for ( ptId = 0;
            ptId < input->GetNumberOfPoints();
            ptId++)
        {
#endif
        input->GetPoint(ptId, pointCoords);
        double voxelGaussianWeight = 1.0 / voxelVolume;

        for (int i = 0; i < 3; ++i)
          {
          if ( sliceData->SampleDimensions[i] > 1 )
            {
            voxelGaussianWeight *= 0.5*(vtkGaussianScalarSplatter_erf((voxMax[i] - pointCoords[i]) / sqrt2sigma) -
                                        vtkGaussianScalarSplatter_erf((voxMin[i] - pointCoords[i]) / sqrt2sigma));
            }
          }

        if (voxelGaussianWeight == 0.0)
          {
          continue; //with for loop
          }
        numberDensity->SetComponent(idx, 0, (numberDensity->GetComponent(idx,0)
                                             + voxelGaussianWeight));
        for(dataArrayIdx = 0;
            dataArrayIdx < outputDataArrays->size() ;
            dataArrayIdx++)
          {
          oDataArray = outputDataArrays->at(dataArrayIdx);
          //assert(inputDataArrays->at(i) != NULL);
          iDataArray = inputDataArrays->at(dataArrayIdx);
          numberOfComponents = oDataArray->GetNumberOfComponents();
          //assert(numberOfComponents==oDataArray->GetNumberOfComponents())
          for(compIdx = 0; compIdx < numberOfComponents; compIdx++)
            {
            oDataArray->SetComponent(idx,compIdx,
                                     (oDataArray->GetComponent(idx,compIdx) +
                                      (iDataArray->GetComponent(ptId,compIdx) *
                                       voxelGaussianWeight)));
            } //for numberOfComponents
          } //for outputDataArrays
        } // for nearby points
      } // for I
    } // for J
  closePoints->Delete();
  pointLocator->Delete();
} // vtkGaussianScalarSplatter_ProcessSlice()

//----------------------------------------------------------------------------
// Compute the size of the sample bounding box automatically from the
// input data.
void vtkGaussianScalarSplatter::ComputeModelBounds(vtkDataSet *input,
                                             vtkImageData *output,
                                             vtkInformation *outInfo)
{
  double *bounds;
  double maxDist;
  int i;

  maxDist = this->StandardDeviation * NUMBER_OF_STD_DEVIATIONS;
  bounds = input->GetBounds();
  for (i = 0; i < 3; i ++)
    {
    if ( this->SampleDimensions[i] > 1 )
      {
      this->ModelBounds[2*i  ] = bounds[2*i  ] - maxDist;
      this->ModelBounds[2*i+1] = bounds[2*i+1] + maxDist;
      }
    else
      {
      this->ModelBounds[2*i  ] = bounds[2*i  ];
      this->ModelBounds[2*i+1] = bounds[2*i+1];
      }
    }

  // Set volume origin and data spacing
  outInfo->Set(vtkDataObject::ORIGIN(),
               this->ModelBounds[0],
               this->ModelBounds[2],
               this->ModelBounds[4]);
  memcpy(this->Origin,outInfo->Get(vtkDataObject::ORIGIN()), sizeof(double)*3);
  output->SetOrigin(this->Origin);

  for (i=0; i<3; i++)
    {
    if ( this->SampleDimensions[i] > 1 )
      {
      this->Spacing[i] = (this->ModelBounds[2*i+1] - this->ModelBounds[2*i])
        / (this->SampleDimensions[i] - 1);
      }
    else
      {
      this->Spacing[i] = 1.0;
      }
    if ( this->Spacing[i] <= 0.0 )
      {
      this->Spacing[i] = 1.0;
      }
    }
  outInfo->Set(vtkDataObject::SPACING(),this->Spacing,3);
  output->SetSpacing(this->Spacing);
}

// Set the dimensions of the sampling structured point set.
void vtkGaussianScalarSplatter::SetSampleDimensions(int i, int j, int k)
{
  int dim[3];

  dim[0] = i;
  dim[1] = j;
  dim[2] = k;

  this->SetSampleDimensions(dim);
}

//----------------------------------------------------------------------------
void vtkGaussianScalarSplatter::SetSampleDimensions(int dim[3])
{
  int dataDim, i;

  vtkDebugMacro(<< " setting SampleDimensions to (" << dim[0] << ","
                << dim[1] << "," << dim[2] << ")");

  if (dim[0] != this->SampleDimensions[0] ||
      dim[1] != this->SampleDimensions[1] ||
      dim[2] != this->SampleDimensions[2] )
    {
    if ( dim[0]<1 || dim[1]<1 || dim[2]<1 )
      {
      vtkErrorMacro (<< "Bad Sample Dimensions, retaining previous values");
      return;
      }

    for (dataDim=0, i=0; i<3 ; i++)
      {
      if (dim[i] > 1)
        {
        dataDim++;
        }
      }

    for ( i=0; i<3; i++)
      {
      this->SampleDimensions[i] = dim[i];
      }

    this->Modified();
    }
}


//----------------------------------------------------------------------------
void vtkGaussianScalarSplatter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Sample Dimensions: ("
               << this->SampleDimensions[0] << ", "
               << this->SampleDimensions[1] << ", "
               << this->SampleDimensions[2] << ")\n";

  os << indent << "StandardDeviation: " << this->StandardDeviation
     << "\n";

  os << indent << "ModelBounds: \n";
  os << indent << "  Xmin,Xmax: (" << this->ModelBounds[0]
     << ", " << this->ModelBounds[1] << ")\n";
  os << indent << "  Ymin,Ymax: (" << this->ModelBounds[2]
     << ", " << this->ModelBounds[3] << ")\n";
  os << indent << "  Zmin,Zmax: (" << this->ModelBounds[4]
     << ", " << this->ModelBounds[5] << ")\n";
}

//----------------------------------------------------------------------------
int vtkGaussianScalarSplatter::FillInputPortInformation(
  int vtkNotUsed(port), vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
  return 1;
}
