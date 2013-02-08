// .NAME vtkVectorComparisonGlyphFilter - brief description
// .SECTION Description
// Description goes here
//
// .SECTION Caveats
//

#ifndef __vtkVectorComparisonGlyphFilter_h
#define __vtkVectorComparisonGlyphFilter_h

#include "vtkFiltersCoreModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"

class VTKFILTERSCORE_EXPORT vtkVectorComparisonGlyphFilter :
  public vtkPolyDataAlgorithm
{
public:
  vtkTypeMacro(vtkVectorComparisonGlyphFilter, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  static vtkVectorComparisonGlyphFilter *New();

protected:
  vtkVectorComparisonGlyphFilter();
  ~vtkVectorComparisonGlyphFilter();

  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  virtual int RequestUpdateExtent(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  virtual int FillInputPortInformation(int, vtkInformation *);

private:
  // Purposely not implemented
  vtkVectorComparisonGlyphFilter(const vtkVectorComparisonGlyphFilter&);
  void operator=(const vtkVectorComparisonGlyphFilter&);

};


#endif // __vtkVectorComparisonGlyphFilter_h
