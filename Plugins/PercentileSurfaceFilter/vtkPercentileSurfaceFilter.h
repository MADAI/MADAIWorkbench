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

#ifndef __vtkPercentileSurfaceFilter_h
#define __vtkPercentileSurfaceFilter_h

#include "vtkPolyDataAlgorithm.h"

class vtkPercentileSurfaceFilter : public vtkPolyDataAlgorithm
{
public:
  vtkTypeMacro(vtkPercentileSurfaceFilter,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  static vtkPercentileSurfaceFilter * New();

  // Description:
  // Set/Get the percentile of points to keep. The percentile is
  // specified in the range [0.0, 1.0].
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

protected:
  double Percentile;

  int RetainLowestValues;

  double MaximumEdgeLength;

  vtkPercentileSurfaceFilter();
  ~vtkPercentileSurfaceFilter();

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

private:
  vtkPercentileSurfaceFilter(const vtkPercentileSurfaceFilter&);  // Not implemented.
  void operator=(const vtkPercentileSurfaceFilter&);  // Not implemented.

};

#endif
