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

#ifndef __vtkRemoveLargeTetrahedraFilter_h
#define __vtkRemoveLargeTetrahedraFilter_h

#include "vtkUnstructuredGridAlgorithm.h"

class vtkRemoveLargeTetrahedraFilter : public vtkUnstructuredGridAlgorithm
{
public:
  vtkTypeMacro(vtkRemoveLargeTetrahedraFilter,vtkUnstructuredGridAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  static vtkRemoveLargeTetrahedraFilter *New();

  // Description:
  // Set/Get the maximum length of an edge in a tetrahedron in the output.
  vtkSetMacro(MaximumEdgeLength, double);
  vtkGetMacro(MaximumEdgeLength, double);

protected:
  vtkRemoveLargeTetrahedraFilter();
  ~vtkRemoveLargeTetrahedraFilter();

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

private:
  vtkRemoveLargeTetrahedraFilter(const vtkRemoveLargeTetrahedraFilter&);  // Not implemented.
  void operator=(const vtkRemoveLargeTetrahedraFilter&);  // Not implemented.

  double MaximumEdgeLength;
};

#endif
