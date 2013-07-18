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

// .NAME vtkMaximizeNormalizedShapeIndexProjectionFilter
// .SECTION Description
// vtkMaximizeNormalizedShapeIndexProjectionFilter

#ifndef __vtkMaximizeNormalizedShapeIndexProjectionFilter_h
#define __vtkMaximizeNormalizedShapeIndexProjectionFilter_h

#include "vtkPolyDataAlgorithm.h"


class vtkMaximizeNormalizedShapeIndexProjectionFilterInternal;

class vtkMaximizeNormalizedShapeIndexProjectionFilter : public vtkPolyDataAlgorithm
{
public:
  vtkTypeMacro(vtkMaximizeNormalizedShapeIndexProjectionFilter,
               vtkPolyDataAlgorithm);
  static vtkMaximizeNormalizedShapeIndexProjectionFilter * New();

  // Description:
  // Set/Get the percentile to keep.
  vtkSetClampMacro(Percentile, double, 0.0, 1.0);
  vtkGetMacro(Percentile, double);

  // Description:
  // Set/Get whether the points with lowest or highest value will be
  // retained. By default, this filter will retain the lowest-valued
  // points.
  vtkSetMacro(RetainLowestValues, int);
  vtkGetMacro(RetainLowestValues, int);
  vtkBooleanMacro(RetainLowestValues, int);

  // Description:
  // Set/Get the maximum length of an edge in a tetrahedron in the output.
  vtkSetMacro(MaximumEdgeLength, double);
  vtkGetMacro(MaximumEdgeLength, double);

  // Description:
  // Get number of point data arrays
  int GetNumberOfPointArrays();

  // Description:
  // Get name of a point data array
  const char* GetPointArrayName(int index);

  // Description:
  // Get/set the status of a point data array
  void SetPointArrayStatus(const char *name, int status);
  int GetPointArrayStatus(const char *name);

protected:
  double Percentile;

  int RetainLowestValues;

  double MaximumEdgeLength;

  vtkMaximizeNormalizedShapeIndexProjectionFilterInternal * Internal;

  vtkMaximizeNormalizedShapeIndexProjectionFilter();
  ~vtkMaximizeNormalizedShapeIndexProjectionFilter();

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

  int FillInputPortInformation(int vtkNotUsed(port), vtkInformation* info);

private:
  vtkMaximizeNormalizedShapeIndexProjectionFilter(const vtkMaximizeNormalizedShapeIndexProjectionFilter&);  // Not implemented.
  void operator=(const vtkMaximizeNormalizedShapeIndexProjectionFilter&);  // Not implemented.

};

#endif
