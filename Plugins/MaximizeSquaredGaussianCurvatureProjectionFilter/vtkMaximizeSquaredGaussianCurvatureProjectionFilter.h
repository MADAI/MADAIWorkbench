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

// .NAME vtkMaximizeSquaredGaussianCurvatureProjectionFilter
// .SECTION Description
// vtkMaximizeSquaredGaussianCurvatureProjectionFilter

#ifndef __vtkMaximizeSquaredGaussianCurvatureProjectionFilter_h
#define __vtkMaximizeSquaredGaussianCurvatureProjectionFilter_h

#include "vtkPolyDataAlgorithm.h"


class vtkMaximizeSquaredGaussianCurvatureProjectionFilter : public vtkPolyDataAlgorithm
{
public:
  vtkTypeMacro(vtkMaximizeSquaredGaussianCurvatureProjectionFilter,
               vtkPolyDataAlgorithm);
  static vtkMaximizeSquaredGaussianCurvatureProjectionFilter * New();

  // Description:
  // Set/Get the percentile to keep.
  vtkSetClampMacro(Percentile, double, 0.0, 1.0);
  vtkGetMacro(Percentile, double);

  // Description:
  // Set/Get the maximum length of an edge in a tetrahedron in the output.
  vtkSetMacro(MaximumEdgeLength, double);
  vtkGetMacro(MaximumEdgeLength, double);

  // Description:
  // Set/Get the number of arrays to be considered.
  // 0 means "all arrays"
  // I used VTK_INT_MAX below instead of VTK_LARGE_INTEGER
  // because VTK_LARGE_INTEGER is not defined for some reason, even
  // when I include vtkTypes.h
  vtkSetClampMacro(Dimensions, int, 0, VTK_INT_MAX);
  vtkGetMacro(Dimensions, int);

protected:
  double Percentile;

  double MaximumEdgeLength;

  int Dimensions;

  vtkMaximizeSquaredGaussianCurvatureProjectionFilter();
  ~vtkMaximizeSquaredGaussianCurvatureProjectionFilter(){}

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

  int FillInputPortInformation(int vtkNotUsed(port), vtkInformation* info);

private:
  vtkMaximizeSquaredGaussianCurvatureProjectionFilter(const vtkMaximizeSquaredGaussianCurvatureProjectionFilter&);  // Not implemented.
  void operator=(const vtkMaximizeSquaredGaussianCurvatureProjectionFilter&);  // Not implemented.

};

#endif
