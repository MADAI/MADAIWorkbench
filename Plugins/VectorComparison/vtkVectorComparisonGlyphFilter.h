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

class VTK_EXPORT vtkVectorComparisonGlyphFilter :
  public vtkPolyDataAlgorithm
{
public:
  vtkTypeMacro(vtkVectorComparisonGlyphFilter, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  static vtkVectorComparisonGlyphFilter *New();

  // Description:
  // Set the scale factor for vector glyphs. Default is 1.0.
  vtkSetMacro(ScaleFactor, double);
  vtkGetMacro(ScaleFactor, double);


  // Description:
  // Set the number of points on the outside of the disk. Default is 8.
  vtkSetMacro(DiskResolution, int);
  vtkGetMacro(DiskResolution, int);

  enum OrientationType
  {
    VTK_HALF_VECTOR=0,
    VTK_FIRST_VECTOR,
    VTK_SECOND_VECTOR,
    VTK_SMALLEST_VECTOR,
    VTK_LARGEST_VECTOR
  };


  // Description:
  // Choose how to align magnitude difference with vector fields
  vtkSetMacro(MagnitudeDifferenceAlignmentMode, int);
  vtkGetMacro(MagnitudeDifferenceAlignmentMode, int);

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

  double ScaleFactor;

  int DiskResolution;

  int MagnitudeDifferenceAlignmentMode;
};


#endif // __vtkVectorComparisonGlyphFilter_h
