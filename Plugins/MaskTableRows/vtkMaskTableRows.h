// .NAME vtkMaskTableRows - Subsamples table rows from a vtkTable.

// .SECTION Description
// This filter enables a sub-sampling of table rows from a vtkTable.
// It is possible to select every n-th row beginning with the first
// row.

#ifndef __vtkMaskTableRows_h
#define __vtkMaskTableRows_h

#include "vtkTableAlgorithm.h"


class VTK_EXPORT vtkMaskTableRows : public vtkTableAlgorithm
{
public:
  vtkTypeMacro(vtkMaskTableRows,vtkTableAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Initialize the filter to select all rows.
  static vtkMaskTableRows *New();

  // Description:
  // Set/get the stride used for sampling every n-th row.
  vtkSetClampMacro(OnRatio, int, 1, VTK_INT_MAX);
  vtkGetMacro(OnRatio, int);

protected:
  vtkMaskTableRows();
  ~vtkMaskTableRows() {};

  virtual int FillInputPortInformation(int port, vtkInformation* info);

  virtual int RequestData(vtkInformation *,
                          vtkInformationVector **,
                          vtkInformationVector *);

  template< class VTK_TT >
  static void SampleColumn(int onRatio,
                           vtkDataArray *input,
                           vtkDataArray *output);

  int OnRatio;

private:
  vtkMaskTableRows(const vtkMaskTableRows&);  // Not implemented.
  void operator=(const vtkMaskTableRows&);  // Not implemented.
};

#endif


