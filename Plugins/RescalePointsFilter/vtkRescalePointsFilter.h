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

#ifndef __vtkRescalePointsFilter_h
#define __vtkRescalePointsFilter_h

#include "vtkPolyDataAlgorithm.h"

class vtkRescalePointsFilter : public vtkPolyDataAlgorithm
{
public:
  vtkTypeMacro(vtkRescalePointsFilter,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  static vtkRescalePointsFilter *New();

protected:
  vtkRescalePointsFilter();
  ~vtkRescalePointsFilter();

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

private:
  vtkRescalePointsFilter(const vtkRescalePointsFilter&);  // Not implemented.
  void operator=(const vtkRescalePointsFilter&);  // Not implemented.

};

#endif
