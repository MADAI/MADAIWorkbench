/*=========================================================================
  Program:   Visualization Toolkit - Binning Filter
  Module:    vtkBinningFilter.cxx

  See README.txt for more information.

  Written 2011 Hal Canary <hal@cs.unc.edu> for the MADAI project
  <http://MADAI.us>.  Based on other VTK Filters, Copyright (c) Ken
  Martin, Will Schroeder, Bill Lorensen.  All rights reserved.

  See VTK_Copyright.txt or http://www.kitware.com/Copyright.htm for details.

    This software is distributed WITHOUT ANY WARRANTY; without even
    the implied warranty of MERCHANTABILITY or FITNESS FOR A
    PARTICULAR PURPOSE.  See the above copyright notice for more
    information.

=========================================================================*/
#include "vtkBinningFilter.h"

#include "vtkObjectFactory.h"
#include "vtkDataSet.h"
#include "vtkInformationVector.h"
#include "vtkInformation.h"
#include "vtkImageData.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkDataArray.h"
#include "vtkPointData.h"
#include "vtkDoubleArray.h"
#include "vtkInformationDoubleVectorKey.h"
#include "vtkMultiThreader.h"

const char DENSITY_STRING[] = "_Density"; //This is the string that is
                              // appended to the vtkDataArray::GetName.

vtkStandardNewMacro(vtkBinningFilter);

vtkBinningFilter::vtkBinningFilter()
{
  this->Dimensions[0] = 50; //these values will hopefully be overwritten.
  this->Dimensions[1] = 50;
  this->Dimensions[2] = 50;
  this->Bounds[0] = 0.0;
  this->Bounds[1] = 50.0;
  this->Bounds[2] = 0.0;
  this->Bounds[3] = 50.0;
  this->Bounds[4] = 0.0;
  this->Bounds[5] = 50.0;
  this->Origin[0] = 0.0;
  this->Origin[1] = 0.0;
  this->Origin[2] = 0.0;
  this->Spacing[0] = 1.0;
  this->Spacing[1] = 1.0;
  this->Spacing[2] = 1.0;
}

//----------------------------------------------------------------------------
// called from RequestInformation() and RequestData()
void vtkBinningFilter::recomputeBoundsSpacingAndOrigin()
{
  // this->GetInput()->GetBounds() may have changed.
  // this->Dimensions[] has changed.
  int i;
  double bounds[6];
  vtkDataSet *ds = vtkDataSet::SafeDownCast(this->GetInput());
  if (ds != NULL)
    {
    ds->GetBounds(bounds);
    for (i=0; i<6; i++)
      {
      this->Bounds[i] = bounds[i];
      }
    }
  // Set volume origin and data spacing
  for (i=0; i<3; i++)
    {
    this->Origin[i] = this->Bounds[2*i];
    if (this->Dimensions[i] > 1)
      {
      this->Spacing[i] = (this->Bounds[2*i+1] - this->Bounds[2*i])
	/ (this->Dimensions[i] - 1);
      }
    else
      {
      this->Spacing[i] = 1.0; // degenerate case.
      }
    }
  return;
}

//----------------------------------------------------------------------------
int vtkBinningFilter::RequestInformation (
  vtkInformation * request,
  vtkInformationVector ** inputVectors,
  vtkInformationVector * outputVector)
{
  vtkInformation * outInfo = outputVector->GetInformationObject(0);
  recomputeBoundsSpacingAndOrigin();
  outInfo->Set(vtkDataObject::ORIGIN(), this->Origin, 3);
  outInfo->Set(vtkDataObject::SPACING(), this->Spacing, 3);
  int i, extent[6];
  for (i=0; i<3; i++)
    {
    extent[2*i] = 0;
    extent[(2*i)+1] = this->Dimensions[i] - 1;
    }
  outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(),
               extent,6);
  return 1;
}



//---------------------------------------------------------------------
inline int clamp(int x, int min, int max)
{
  return ((x < min) ? min : ((x > max) ? max : x));
}
//---------------------------------------------------------------------
// data structure shared by each thread.
class BinningThreadData {
public:
  vtkDataSet * input; //pointer to input Data Set
                      // I need the input->GetPoint() function.
  double Origin[3];  // copied from this->Origin,
  double Spacing[3];             // this->Spacing, and
  int Dimensions[3];             // this->Dimensions
  int numberOfInputArrays;
  vtkDataArray ** inputDataArrays; // array of size [numberOfInputArrays]
                                   // of pointers to vtkDataArrays.
  vtkDataArray *** outputDataArrays; // array of array of pointers.
  // array dimensions = [numberOfThreads][numberOfInputArrays]
};
//---------------------------------------------------------------------
static VTK_THREAD_RETURN_TYPE threadedExecute( void * arg );
//---------------------------------------------------------------------
int vtkBinningFilter::RequestData(
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
  output->SetOrigin(this->Origin);
  output->SetSpacing(this->Spacing);
  output->SetDimensions(this->Dimensions);

  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkDataSet *input = vtkDataSet::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));

  int numberOfInputDataArrays = input->GetPointData()->GetNumberOfArrays();
  int i, inputDataArraysIndex, dataType, dataArrayIndex;
  int numberOfThreads, inputNumberOfPoints, outputNumberOfPoints;
  int threadId, numberOfComponents, numberOfTuples, tuple, component;
  vtkMultiThreader * threader;
  BinningThreadData thdata;
  vtkDataArray * a1, * a2;
  double value;
  std::vector<int> inputDataArraysIndices; //temporary list of indices
					   //we care about.

  // copy pointers and values.  threadedExecute() is a static function
  // so that I am SURE I don't accidentally step on any shared data.
  thdata.input = input;
  for (i = 0; i < 3; i++)
    {
    thdata.Origin[i] = this->Origin[i];
    thdata.Spacing[i] = this->Spacing[i];
    thdata.Dimensions[i] = this->Dimensions[i];
    }

  inputNumberOfPoints = input->GetNumberOfPoints();
  if (inputNumberOfPoints < 1 )
    {
    return 1;   //  No points are available.
    }
  outputNumberOfPoints = (this->Dimensions[0] *
			  this->Dimensions[1] *
			  this->Dimensions[2]);
  bool usethreads = true;
  if (inputNumberOfPoints < outputNumberOfPoints) {
    usethreads = false;
    // It would take longer to recombine the thread's output than to
    // compute it in the first place.  It might be better to stay
    // single-threaded for some edge cases, but I'm sure that when
    // inputNumberOfPoints is *very* much greater than
    // outputNumberOfPoints, multithreading can help.
  }

  thdata.numberOfInputArrays = 0;

  // Find out which data arrays I want to work with.
  for (inputDataArraysIndex = 0;
       inputDataArraysIndex < numberOfInputDataArrays;
       inputDataArraysIndex++)
    {
    dataType = input->GetPointData()
    	->GetArray(inputDataArraysIndex)->GetDataType();
    if ((dataType == VTK_DOUBLE) || (dataType == VTK_FLOAT))
      //|| (dataType == VTK_INT   ) || (dataType == VTK_UNSIGNED_INT)
      //|| (dataType == VTK_LONG  ) || (dataType == VTK_UNSIGNED_LONG))
      {
      thdata.numberOfInputArrays++;
      inputDataArraysIndices.push_back(inputDataArraysIndex);
      }
    }

  // Make a list of data arrays to pass to the threads.
  thdata.inputDataArrays
    = new vtkDataArray * [thdata.numberOfInputArrays];//FIXME
  for (dataArrayIndex = 0;
       dataArrayIndex < thdata.numberOfInputArrays;
       dataArrayIndex++)
    {
    thdata.inputDataArrays[dataArrayIndex]
      = input->GetPointData()->GetArray(
	  inputDataArraysIndices.at(dataArrayIndex));
    }

  // Create thread manager object.
  threader = vtkMultiThreader::New();
  if (usethreads)
    {
    numberOfThreads = vtkMultiThreader::GetGlobalDefaultNumberOfThreads();
    vtkMultiThreader::SetGlobalMaximumNumberOfThreads(numberOfThreads);
    if (numberOfThreads > inputNumberOfPoints)
      {
      numberOfThreads = inputNumberOfPoints;
      }
    }
  else
    {
    numberOfThreads = 1;
    }
  threader->SetNumberOfThreads(numberOfThreads);

  // this is the array that will hold the data the threads will return.
  thdata.outputDataArrays = new vtkDataArray ** [numberOfThreads];

  threader->SetSingleMethod(threadedExecute,
			static_cast<void *>(&thdata));
  threader->SingleMethodExecute();
  threader->Delete();

  delete thdata.inputDataArrays; // no longer need these pointers.

  // We should do a map reduction, but how much speed will that get us?
  // Start at 1, becuase we fold into array 0.
  for (threadId = 1; threadId < numberOfThreads; threadId++)
    {
    // (thdata.numberOfInputArrays + 1) because we use numberDensity.
    for (dataArrayIndex = 0;
	 dataArrayIndex < (thdata.numberOfInputArrays + 1);
	 dataArrayIndex++)
      {
      a1 = thdata.outputDataArrays[       0][dataArrayIndex];
      a2 = thdata.outputDataArrays[threadId][dataArrayIndex];
      //assert((a1 != NULL) && (a2 != NULL)));
      numberOfComponents = a1->GetNumberOfComponents();
      numberOfTuples = a1->GetNumberOfTuples();
      //assert(numberOfComponents == a2->GetNumberOfComponents());
      //assert(numberOfTuples == a1->GetNumberOfTuples());
      for (tuple = 0 ; tuple < numberOfTuples; tuple++)
	{
	// 95% of the time, numberOfComponents==1.  How do I optimize?
	for (component = 0; component < numberOfComponents; component++)
	  {
	  value = (a1->GetComponent (tuple, component) +
		   a2->GetComponent (tuple, component));
	  a1->SetComponent(tuple, component, value);
	  }
	}
      // We are now done with this dataArray.
      thdata.outputDataArrays[threadId][dataArrayIndex]->Delete();
      }
    // We are now done with this array.
    delete thdata.outputDataArrays[threadId];
    }
  // add dataArrays to the output dataSet.
  for (dataArrayIndex = 0;
       dataArrayIndex < (thdata.numberOfInputArrays + 1);
       dataArrayIndex++)
    {
    output->GetPointData()->AddArray(
      thdata.outputDataArrays[0][dataArrayIndex]);
    }
  delete thdata.outputDataArrays;
  return 1;
}

static VTK_THREAD_RETURN_TYPE threadedExecute( void * arg )
{
  vtkMultiThreader::ThreadInfo * threadInfo;
  BinningThreadData * threadData;
  double pointCoords[3];
  int bin[3], imageIndex,  ptId, dataArraysIndex;
  int i, compIdx, numberOfComponents;
  char * oldname, *newname;
  vtkDataArray * iDataArray, * oDataArray;
  vtkDoubleArray * numberDensity;

  threadInfo = static_cast<vtkMultiThreader::ThreadInfo *>(arg);
  threadData = static_cast<BinningThreadData *>(threadInfo->UserData);
  int threadId = threadInfo->ThreadID;
  int threadCount = threadInfo->NumberOfThreads;

  double inverseVoxelVolume = 1 / (threadData->Spacing[0] *
				   threadData->Spacing[1] *
				   threadData->Spacing[2]);
  int sliceSize = threadData->Dimensions[0] * threadData->Dimensions[1];

  int numNewPts = (threadData->Dimensions[0] *
		   threadData->Dimensions[1] *
		   threadData->Dimensions[2]);

  int numberPoints = threadData->input->GetNumberOfPoints();
  int blocksize = 1 + ((numberPoints - 1) / threadCount);
  int firstPointId = threadId * blocksize;
  int lastPointId = firstPointId + blocksize;
  if (lastPointId >= numberPoints) //last thread
    {
    lastPointId = numberPoints;
    }
  numberDensity = vtkDoubleArray::New();
  numberDensity->SetNumberOfComponents(1);
  numberDensity->SetName("NumberDensity");
  // PLEASE NOTE: Summing up all the Number Density and multiplying
  // by the voxel volume should return the original number of points.
  numberDensity->SetNumberOfTuples(numNewPts);
  for (imageIndex=0; imageIndex<numNewPts; imageIndex++)
    {
    numberDensity->SetComponent(imageIndex, 0, 0.0); //initialize to ZERO
    }
  //create output arrays after input data arrays
  // loop through the input data arrays to make output
  threadData->outputDataArrays[threadId]
    = new vtkDataArray * [threadData->numberOfInputArrays + 1];
  // [+ 1] for numberDensity array.
  threadData->outputDataArrays[threadId][threadData->numberOfInputArrays]
    = numberDensity;
  for (dataArraysIndex = 0;
       dataArraysIndex < threadData->numberOfInputArrays;
       dataArraysIndex++)
    {
    iDataArray = threadData->inputDataArrays[dataArraysIndex];
    oDataArray = vtkDataArray::CreateDataArray(iDataArray->GetDataType());
    if (threadId == 0)
      {
      //oDataArray->SetName(iDataArray->GetName() + DENSITY_STRING);
      oldname = iDataArray->GetName();
      newname = new char [(strlen(oldname) + sizeof(DENSITY_STRING))];
      strcpy(newname, oldname);
      strcpy(newname + strlen(oldname), DENSITY_STRING);
      oDataArray->SetName(newname);
      delete newname;
      } // else we don't care.
    numberOfComponents = iDataArray->GetNumberOfComponents();
    oDataArray->SetNumberOfComponents(numberOfComponents);
    oDataArray->SetNumberOfTuples(numNewPts);// allocate enough memory
    for (imageIndex = 0; imageIndex < numNewPts; imageIndex++)
      {
      for(compIdx = 0; compIdx < numberOfComponents; compIdx++)
	{
	oDataArray->SetComponent(imageIndex, 0, 0.0); //initialize to ZERO
	}
      }
    threadData->outputDataArrays[threadId][dataArraysIndex] = oDataArray;
    }
  for (ptId = firstPointId; ptId < lastPointId; ptId ++)
    {
    threadData->input->GetPoint(ptId, pointCoords);
    //first convert pointCoords to integer vtkImageData coordinates
    for(i=0; i<3; i++)
      {
      if (isnan(pointCoords[i]))
	{
	bin[i] = 0;  //	numerrors++;
	continue;
	}
      bin[i] = static_cast<int>(floor(
        (pointCoords[i] - threadData->Origin[i]) /
          threadData->Spacing[i]));
      }
    bin[0] = clamp(bin[0], 0 , threadData->Dimensions[0] - 1);
    bin[1] = clamp(bin[1], 0 , threadData->Dimensions[1] - 1);
    bin[2] = clamp(bin[2], 0 , threadData->Dimensions[2] - 1);

    //convert vtkImageData [i,j,k] coords to single vtkDataArray index.
    imageIndex = (bin[0] +
	   (bin[1] * threadData->Dimensions[0]) +
	   (bin[2] * sliceSize));
    numberDensity->SetComponent(imageIndex, 0,
		(numberDensity->GetComponent(imageIndex,0)
			 + inverseVoxelVolume));
    for(dataArraysIndex = 0;
	dataArraysIndex < threadData->numberOfInputArrays;
	dataArraysIndex++)
      {
      oDataArray = threadData->outputDataArrays[threadId][dataArraysIndex];
      iDataArray = threadData->inputDataArrays[dataArraysIndex];
      //assert((oDataArray != NULL) && (iDataArray != NULL));
      numberOfComponents = oDataArray->GetNumberOfComponents();
      //assert(numberOfComponents==oDataArray->GetNumberOfComponents())
      for(compIdx = 0; compIdx < numberOfComponents; compIdx++)
	{
	oDataArray->SetComponent(imageIndex,compIdx,
				 (oDataArray->GetComponent(imageIndex,compIdx) +
				  (iDataArray->GetComponent(ptId,compIdx) *
				   inverseVoxelVolume)));
	} //for numberOfComponents
      } //for outputDataArrays
    } // for ptId
  return VTK_THREAD_RETURN_VALUE;
}


void vtkBinningFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Dimensions: ("
               << this->Dimensions[0] << ", "
               << this->Dimensions[1] << ", "
               << this->Dimensions[2] << ")\n";
  os << indent << "Origin: ("
               << this->Origin[0] << ", "
               << this->Origin[1] << ", "
               << this->Origin[2] << ")\n";
  os << indent << "Bounds: \n";
  os << indent << "  Xmin,Xmax: (" << this->Bounds[0]
     << ", " << this->Bounds[1] << ")\n";
  os << indent << "  Ymin,Ymax: (" << this->Bounds[2]
     << ", " << this->Bounds[3] << ")\n";
  os << indent << "  Zmin,Zmax: (" << this->Bounds[4]
     << ", " << this->Bounds[5] << ")\n";
}

void vtkBinningFilter::SetDimensions(int i, int j, int k)
{
  int dim[3];
  dim[0] = i;
  dim[1] = j;
  dim[2] = k;
  this->SetDimensions(dim);
}

//----------------------------------------------------------------------------
void vtkBinningFilter::SetDimensions(int dim[3])
{
  int i;
  for ( i=0; i<3; i++)
    {
    if (dim[i] < 1)
      {
      vtkErrorMacro(<<"Sample dimensions must be positive!");
      return;
      }
    this->Dimensions[i] = dim[i];
    }
  recomputeBoundsSpacingAndOrigin();
  this->Modified();
}

//----------------------------------------------------------------------------
int vtkBinningFilter::FillInputPortInformation(
  int vtkNotUsed(port), vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
  return 1;
}
