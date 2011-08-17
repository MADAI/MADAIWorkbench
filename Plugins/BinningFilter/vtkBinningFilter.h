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

// .NAME vtkBinningFilter - splat points into a volume by distributing them into bins and summing up the data arrays in each bin

// .SECTION Description
// vtkBinningFilter is a filter that injects values from input points
// into a structured points (volume) dataset. As each point is
// injected, it contributes to a single voxel, or "bin."
//
// This class is typically used to convert point-valued distributions into
// a volume representation. The volume is then usually iso-surfaced or
// volume rendered to generate a visualization. It can be used to create
// surfaces from point distributions, or to create structure (i.e.,
// topology) when none exists.
//
// .SECTION Caveats
// The input to this filter is any dataset type. This filter can be used
// to resample any form of data, i.e., the input data need not be
// unstructured.
//
// Some voxels may never receive a contribution during the splatting process.
// The final value of these points is 0.
//
//
// .SECTION See Also
// vtkShepardMethod, vtkGaussianSplatter


#ifndef __vtkBinningFilter_h
#define __vtkBinningFilter_h

#include "vtkImageAlgorithm.h"

class VTK_EXPORT vtkBinningFilter : public vtkImageAlgorithm
{
public:
  vtkTypeMacro(vtkBinningFilter,vtkImageAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Construct object with Dimensions = (50,50,50)
  static vtkBinningFilter *New();

  // Description:
  // Set / get the dimensions of the output image point set.
  void SetDimensions(int i, int j, int k);
  void SetDimensions(int dim[3]);
  vtkGetVectorMacro(Dimensions,int,3);

protected:
  vtkBinningFilter();
  ~vtkBinningFilter() {};

  virtual int FillInputPortInformation(int port, vtkInformation* info);
  virtual int RequestInformation (vtkInformation *,
                                  vtkInformationVector **,
                                  vtkInformationVector *);
  virtual int RequestData(vtkInformation *,
                          vtkInformationVector **,
                          vtkInformationVector *);

  int Dimensions[3]; // dimensions of volume to bin into
  double Bounds[6]; // bounding box image
  double Origin[3];
  double Spacing[3];

//ETX
private:
  void recomputeBoundsSpacingAndOrigin();
  vtkBinningFilter(const vtkBinningFilter&);  // Not implemented.
  void operator=(const vtkBinningFilter&);  // Not implemented.
};
#endif
