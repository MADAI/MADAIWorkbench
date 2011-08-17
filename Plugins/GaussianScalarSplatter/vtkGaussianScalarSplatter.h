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

// .NAME vtkGaussianScalarSplatter - splat points into a volume with a spherical, Gaussian distribution

// .SECTION Description
// vtkGaussianScalarSplatter is a filter that injects input points into a
// structured points (volume) dataset. As each point is injected, it "splats"
// or distributes values to nearby voxels. Data is distributed using an
// spherical, Gaussian distribution function. The distribution function is
// modified using scalar values (expands distribution).
//
// In general, the normalized Gaussian distribution function f(x) around
// a given splat point p is given by
//
//     f(x) = A * exp((r ** 2) / (-2 * StandardDeviation)),
//     where A = 1.0 / (StandardDeviation * sqrt(2 * PI))
//
// where x is the current voxel sample point; r is the distance |x-p|.
// f(x) can be multiplied by the scalar value of the point p that is
// currently being splatted.  If f(x) is not multiplied by anything,
// it gives NumberDensity.
//
// In practice, each point in the output is the integral of f(x) over
// the surrounding volume (voxel), divided by the volume of the voxel.
// These integrals are calulated using the erf() function.
//
// This class is typically used to convert point-valued distributions into
// a volume representation. The volume is then usually iso-surfaced or
// volume rendered to generate a visualization. It can be used to create
// surfaces from point distributions, or to create structure (i.e.,
// topology) when none exists.

// .SECTION Caveats
// The input to this filter is any dataset type. This filter can be used
// to resample any form of data, i.e., the input data need not be
// unstructured.
//
// Some voxels may never receive a contribution during the splatting process.
// The final value of these points is 0.

// .SECTION See Also
// vtkShepardMethod, vtkGaussianSplatter

#ifndef __vtkGaussianScalarSplatter_h
#define __vtkGaussianScalarSplatter_h

#include "vtkImageAlgorithm.h"

class vtkDoubleArray;

class VTK_EXPORT vtkGaussianScalarSplatter : public vtkImageAlgorithm
{
public:
  vtkTypeMacro(vtkGaussianScalarSplatter,vtkImageAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:

  // Construct object with SampleDimensions = (50,50,50) and
  // StandardDeviation = DEFAULT_STANDARD_DEVIATION.
  static vtkGaussianScalarSplatter *New();

  // Description:
  // Set / get the dimensions of the sampling structured point set. Higher
  // values produce better results but are much slower.
  void SetSampleDimensions(int i, int j, int k);
  void SetSampleDimensions(int dim[3]);
  vtkGetVectorMacro(SampleDimensions,int,3);

  // Description:
  // Set / get the Standard Deviation propagation of the Gaussian
  // Function of the splat. Smaller numbers greatly reduce execution time.
  vtkSetClampMacro(StandardDeviation,double,0.0,VTK_DOUBLE_MAX);
  vtkGetMacro(StandardDeviation,double);

  /* // Description: */
  /* // Set / get the (xmin,xmax, ymin,ymax, zmin,zmax) bounding box in which */
  /* // the sampling is performed. If any of the (min,max) bounds values are */
  /* // min >= max, then the bounds will be computed automatically from the input */
  /* // data. Otherwise, the user-specified bounds will be used. */
  /* vtkSetVector6Macro(ModelBounds,double); */
  /* vtkGetVectorMacro(ModelBounds,double,6); */

protected:
  vtkGaussianScalarSplatter();
  ~vtkGaussianScalarSplatter() {};

  virtual int FillInputPortInformation(int port, vtkInformation* info);
  virtual int RequestInformation (vtkInformation *,
                                  vtkInformationVector **,
                                  vtkInformationVector *);
  virtual int RequestData(vtkInformation *,
                          vtkInformationVector **,
                          vtkInformationVector *);

  // Description:
  // Compute the size of the sample bounding box automatically from the
  // input data. This is an internal helper function.
  void ComputeModelBounds(vtkDataSet *input, vtkImageData *output,
                          vtkInformation *outInfo);

  static const double DEFAULT_STANDARD_DEVIATION;
  int SampleDimensions[3]; // dimensions of volume to splat into
  double StandardDeviation; // Standard Deviation of the Gaussians
  double ModelBounds[6]; // bounding box of splatting dimensions
  double Origin[3];
  double Spacing[3];

//ETX

private:
  vtkGaussianScalarSplatter(const vtkGaussianScalarSplatter&);  // Not implemented.
  void operator=(const vtkGaussianScalarSplatter&);  // Not implemented.
};

#endif


