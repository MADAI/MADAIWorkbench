#include "vtkMaskTableRows.h"

#include "vtkDataArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkTable.h"

vtkStandardNewMacro(vtkMaskTableRows);

//----------------------------------------------------------------------------
vtkMaskTableRows::vtkMaskTableRows()
{
  this->OnRatio = 1;
}

//----------------------------------------------------------------------------
int vtkMaskTableRows::RequestData(
  vtkInformation* vtkNotUsed( request ),
  vtkInformationVector** inputVector,
  vtkInformationVector* outputVector)
{
  // get the data object
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  vtkTable *output = vtkTable::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkTable *input = vtkTable::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));

  if ( input->GetNumberOfRows() == 0 )
    {
    return 1;
    }

  vtkIdType rows = input->GetNumberOfRows();
  vtkIdType columns = input->GetNumberOfColumns();
  vtkIdType outputRows = rows / this->OnRatio;

  for ( vtkIdType j = 0; j < columns; ++j )
    {
    vtkDataArray *inColumn =
      vtkDataArray::SafeDownCast( input->GetColumn( j ) );
    vtkDataArray *outColumn =
      vtkDataArray::SafeDownCast( inColumn->NewInstance() );
    outColumn->SetName( inColumn->GetName() );
    outColumn->SetNumberOfComponents( 1 );
    outColumn->SetNumberOfTuples( outputRows );
    switch ( outColumn->GetDataType() )
      {
      vtkExtendedTemplateMacro(
        SampleColumn< VTK_TT >( this->OnRatio, inColumn, outColumn ) );
      }
    output->AddColumn( outColumn );
    outColumn->Delete();
    }

  return 1;
}

//----------------------------------------------------------------------------
template< class VTK_TT >
void vtkMaskTableRows::SampleColumn(int onRatio,
                                    vtkDataArray *input,
                                    vtkDataArray *output)
{
  VTK_TT *inPtr  = static_cast< VTK_TT* >( input->GetVoidPointer( 0 ) );
  VTK_TT *outPtr = static_cast< VTK_TT* >( output->GetVoidPointer( 0 ) );
    
  for ( vtkIdType i = 0; i < output->GetNumberOfTuples(); ++i )
    {
    outPtr[i] = inPtr[ i*onRatio ];
    }
}

//----------------------------------------------------------------------------
void vtkMaskTableRows::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "OnRatio: (" << this->OnRatio << ")\n";
}

//----------------------------------------------------------------------------
int vtkMaskTableRows::FillInputPortInformation(
  int vtkNotUsed(port), vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkTable");
  return 1;
}
