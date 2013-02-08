#include "vtkVectorComparisonGlyphFilter.h"

#include "vtkAlgorithm.h"
#include "vtkInformation.h"

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
  if (port == 0 || port == 1)
    {
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
    info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 0);
    return 1;
    }
    
  return 0;
}
