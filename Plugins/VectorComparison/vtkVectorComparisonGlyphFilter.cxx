#include "vtkVectorComparisonGlyphFilter.h"

vtkStandardNewMacro(vtkVectorComparisonGlyphFilter);


//----------------------------------------------------------------------------
vtkVectorComparisonGlyphFilter::vtkVectorComparisonGlyphFilter()
{

}

//----------------------------------------------------------------------------
vtkVectorComparisonGlyphFilter::~vtkVectorComparisonGlyphFilter()
{

}

//----------------------------------------------------------------------------
void vtkVectorComparisonGlyphFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
int vtkVectorComparisonGlyphFilter::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  return 1;
}

//----------------------------------------------------------------------------
int vtkVectorComparisonGlyphFilter::RequestUpdateExtent(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  return 1;
}

//----------------------------------------------------------------------------
int vtkVectorComparisonGlyphFilter::FillInputPortInformation(int port, vtkInformation *info)
{

  return 1;
}
