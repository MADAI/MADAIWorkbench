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
#include "vtkImageData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkPointData.h"
#include "vtkPointLocator.h"
#include "vtkPoints.h"
#include "vtkIdList.h"
#include "vtkMultiThreader.h"

const double NUMBER_OF_STD_DEVIATIONS = 4.0; 
const char DENSITY_STRING [] = "_Density";
const double vtkGaussianScalarSplatter::DEFAULT_STANDARD_DEVIATION = 1.0;

// This is public domain code from John D. Cook based on
// the formula 7.1.26 in the Handbook of Mathematical Functions
// by Abramowitz and Stegun. http://www.johndcook.com/cpp_erf.html
// It really should be put somewhere better than here, but this will
// work for now.
double erf(double x)
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

class sliceDataType {
public:
  vtkDataSet * input;
  vtkDoubleArray * numberDensity;
  std::vector<vtkDataArray *> * outputDataArrays;
  std::vector<vtkDataArray *> * inputDataArrays;
  double voxelVolume;
  double sqrt2sigma;
  double radius;
  double origin[3];
  double spacing[3];
  int sampleDimensions[3];
};
static void processSlice(sliceDataType * sliceData, int sliceIndex);
static VTK_THREAD_RETURN_TYPE threadedExecute( void * arg );

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
    this->Spacing[i] = (this->ModelBounds[2*i+1] - this->ModelBounds[2*i])
      / (this->SampleDimensions[i] - 1);
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

  std::vector<vtkDataArray *> outputDataArrays; // pointers to
	// the arrays I'm going to splat.
  std::vector<vtkDataArray *> inputDataArrays; // pointers to
	// the arrays I'm splatting into

  // sliceData is a chunck of memory containing information I'm going
  // to pass to the processSlice() function.
  sliceDataType sliceData;
  sliceData.input = input;
  sliceData.outputDataArrays = (& outputDataArrays);
  sliceData.inputDataArrays = (& inputDataArrays); 

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
    sliceData.origin[i] = this->Origin[i];
    sliceData.spacing[i] = this->Spacing[i];
    sliceData.sampleDimensions[i] = this->SampleDimensions[i];
    if (largest_dim > this->Spacing[i])
      {
      largest_dim = this->Spacing[i];
      }
    }
  sliceData.voxelVolume = (this->Spacing[0] *
			   this->Spacing[1] *
			   this->Spacing[2]);
  sliceData.sqrt2sigma = (sqrt(2.0) * this->StandardDeviation);
  sliceData.radius = ((this->StandardDeviation * NUMBER_OF_STD_DEVIATIONS)
		      + (largest_dim / 2.0));

  // Inserting a new array.
  sliceData.numberDensity = vtkDoubleArray::New();
  sliceData.numberDensity->SetNumberOfComponents(1);
  sliceData.numberDensity->SetName("NumberDensity");
  // PLEASE NOTE: Summing up all the Number Density and multiplying
  // by the voxel volume should return the original number of poitns.
  sliceData.numberDensity->SetNumberOfTuples(numNewPts);
  output->GetPointData()->AddArray(sliceData.numberDensity);
  for (idx=0; idx<numNewPts; idx++)
    {
    sliceData.numberDensity->SetComponent(idx, 0, 0.0); //initialize to ZERO
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

    //oDataArray->SetName(iDataArray->GetName() + DENSITY_STRING);
    oldname = iDataArray->GetName();
    newname = new char [(strlen(oldname) + sizeof(DENSITY_STRING))];
    strcpy(newname, oldname);
    strcpy(newname + strlen(oldname), DENSITY_STRING);
    oDataArray->SetName(newname);
    delete newname;

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

  //initilize multiple threads
  vtkMultiThreader * threader = vtkMultiThreader::New();
  int numberOfThreads = vtkMultiThreader::GetGlobalDefaultNumberOfThreads();
  vtkMultiThreader::SetGlobalMaximumNumberOfThreads(numberOfThreads);
  if (numberOfThreads > this->SampleDimensions[2])
    {
    numberOfThreads = this->SampleDimensions[2];
    }
  threader->SetNumberOfThreads(numberOfThreads);
  threader->SetSingleMethod(threadedExecute, 
			    static_cast<void *>(&sliceData));
  threader->SingleMethodExecute();
  threader->Delete();

  vtkDebugMacro(<< "Splatted " << numPts << " points");
  return 1;
}

//----------------------------------------------------------------------------
static VTK_THREAD_RETURN_TYPE threadedExecute( void * arg )
{
  vtkMultiThreader::ThreadInfo * threadInfo;
  sliceDataType * threadData;
  int sliceIndex, threadId, threadCount, numberSlices;
  threadInfo = static_cast<vtkMultiThreader::ThreadInfo *>(arg);
  threadData = static_cast<sliceDataType *>(threadInfo->UserData);
  threadId = threadInfo->ThreadID;
  threadCount = threadInfo->NumberOfThreads;
  numberSlices = threadData->sampleDimensions[2];

  for (sliceIndex = threadId;
       sliceIndex < numberSlices; 
       sliceIndex += threadCount)
    {
    processSlice(threadData, sliceIndex);
    }
  return VTK_THREAD_RETURN_VALUE;
}


//----------------------------------------------------------------------------
// Static Function called by vtkGaussianScalarSplatter::RequestData()
// this function will modify 
//   sliceData->outputDataArrays->at(*)->GetComponent(index,*)
//   sliceData->numberDensity->GetComponent(index,0) 
//    (for the range of indices corresponding to this slice)
// Other values will be left alone.
static void processSlice(sliceDataType * sliceData, int sliceIndex) 
{
  double voxLoc[3], voxMin[3], voxMax[3];
  int vox[2]; // vox[2] would be sliceIndex;

  vtkDataSet * input = sliceData->input;

  vtkPointLocator * pointLocator = vtkPointLocator::New();
  pointLocator->SetDataSet(input);

  vtkDoubleArray * numberDensity = sliceData->numberDensity;
  std::vector<vtkDataArray *> * outputDataArrays = sliceData->outputDataArrays;
  std::vector<vtkDataArray *> * inputDataArrays = sliceData->inputDataArrays;
  double sqrt2sigma = sliceData->sqrt2sigma;

  voxLoc[2] = sliceData->origin[2] + (sliceData->spacing[2] * sliceIndex);
  voxMin[2] = voxLoc[2] - (sliceData->spacing[2] / 2);
  voxMax[2] = voxLoc[2] + (sliceData->spacing[2] / 2);

  vtkIdList * closePoints = vtkIdList::New();
  int sliceSize = (sliceData->sampleDimensions[0] *
		   sliceData->sampleDimensions[1]);
  double * pointCoords; // double[3] coords of a point
  double voxGausWeight;
  int ptId, nearPoint;
  int dataArrayIdx,compIdx, numberOfComponents, idx;
  vtkDataArray * oDataArray, * iDataArray;

  long int erfsCounted = 0;

  for (vox[1] = 0, 
	 voxLoc[1] = sliceData->origin[1],
	 voxMin[1] = voxLoc[1] - (sliceData->spacing[1] / 2),
	 voxMax[1] = voxLoc[1] + (sliceData->spacing[1] / 2);
       vox[1] < sliceData->sampleDimensions[1];
       vox[1]++, 
	 voxMin[1] = voxMax[1],
	 voxMax[1] += sliceData->spacing[1],
	 voxLoc[1] += sliceData->spacing[1])
    {
    idx = (vox[1] * sliceData->sampleDimensions[0]) + (sliceIndex * sliceSize);
    for (vox[0] = 0, 
	   voxLoc[0] = sliceData->origin[0],
	   voxMin[0] = voxLoc[0] - (sliceData->spacing[0] / 2),
	   voxMax[0] = voxLoc[0] + (sliceData->spacing[0] / 2);
	 vox[0] < sliceData->sampleDimensions[0];
	 vox[0]++, 
	   voxMin[0] = voxMax[0],
	   voxMax[0] += sliceData->spacing[0],
	   voxLoc[0] += sliceData->spacing[0],
	   idx++)
      {
      closePoints->Reset();
      pointLocator->FindPointsWithinRadius(sliceData->radius,
					   voxLoc, closePoints);
      erfsCounted += closePoints->GetNumberOfIds();
      for (nearPoint = 0; 
	   nearPoint < closePoints->GetNumberOfIds();
	   nearPoint++)
	{
	ptId = closePoints->GetId(nearPoint);
	pointCoords = input->GetPoint(ptId);
	voxGausWeight = (
	  (erf((voxMax[0] - pointCoords[0]) / sqrt2sigma) -
	   erf((voxMin[0] - pointCoords[0]) / sqrt2sigma)) *
	  (erf((voxMax[1] - pointCoords[1]) / sqrt2sigma) -
	   erf((voxMin[1] - pointCoords[1]) / sqrt2sigma)) *
	  (erf((voxMax[2] - pointCoords[2]) / sqrt2sigma) -
	   erf((voxMin[2] - pointCoords[2]) / sqrt2sigma)) / 8.0);
	voxGausWeight /= sliceData->voxelVolume;

	if (voxGausWeight == 0.0)
	  {
	  continue; //with for loop
	  }
	numberDensity->SetComponent(idx, 0,
	  (numberDensity->GetComponent(idx,0)
	   + voxGausWeight));
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
		voxGausWeight)));
	    } //for numberOfComponents
	  } //for outputDataArrays
	} // for nearby points
      } // for I
    } // for J
  closePoints->Delete();
  pointLocator->Delete();
} // processSlice()

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
  for (i = 0; i < 6; i += 2)
    {
    this->ModelBounds[i] = bounds[i] - maxDist;
    }
  for (i = 1; i < 6; i += 2)
    {
    this->ModelBounds[i] = bounds[i] + maxDist;
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
    this->Spacing[i] = (this->ModelBounds[2*i+1] - this->ModelBounds[2*i])
      / (this->SampleDimensions[i] - 1);
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

    if ( dataDim  < 3 )
      {
      vtkErrorMacro(<<"Sample dimensions must define a volume!");
      return;
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
