/*********************************************************************
Gauss Splatter - A stand-alone VTK splatter conversion program.
Copyright 2011-2012, The University of North Carolina at Chapel Hill.

This software was written in 2011-2012 by Hal Canary <cs.unc.edu/~hal>.
while working for the MADAI project <madai.us/>.

This software is distributed under the same license terms as VTK.

See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

    This software is distributed WITHOUT ANY WARRANTY; without even
    the implied warranty of MERCHANTABILITY or FITNESS FOR A
    PARTICULAR PURPOSE.  See the above copyright notice for more
    information.
*********************************************************************/
#include <iostream>
#include <vtkSmartPointer.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkXMLImageDataWriter.h>
#include "../vtkGaussianScalarSplatter.h"

int splat(const char* inFile,
	  const char* outFile,
	  float standardDeviation,
	  int sampleDimensionsX,
	  int sampleDimensionsY,
	  int sampleDimensionsZ) {
  vtkSmartPointer<vtkXMLPolyDataReader> reader =
    vtkSmartPointer<vtkXMLPolyDataReader>::New();
  reader->SetFileName(inFile);

  vtkSmartPointer<vtkGaussianScalarSplatter> splat;
  splat = vtkGaussianScalarSplatter::New();
  splat->SetStandardDeviation(standardDeviation);
  splat->SetSampleDimensions(sampleDimensionsX,
    sampleDimensionsY, sampleDimensionsZ);
  splat->SetInputConnection(reader->GetOutputPort());

  vtkSmartPointer<vtkXMLImageDataWriter> writer =
    vtkSmartPointer<vtkXMLImageDataWriter>::New();
  writer->SetFileName(outFile);
  writer->SetInputConnection(splat->GetOutputPort());
  writer->Write();
  return 0;
}

int main( int argc, char *argv[] ) {
  int sampleDimensionsX, sampleDimensionsY, sampleDimensionsZ;
  float standardDeviation;
  if (argc < 5) {
    std::cerr
      << "\nUsage:\n    " << argv[0] << " INFILE.vtp"
      " OUTFILE.vti STD_DEV X_DIM [Y_DIM] [Z_DIM]\n\n";
    return 1;
  }
  standardDeviation = atof(argv[3]);
  sampleDimensionsX = atoi(argv[4]);
  if (argc > 5) {
    sampleDimensionsY = atoi(argv[5]);
    if (argc > 6) {
      sampleDimensionsZ = atoi(argv[6]);
    } else {
      sampleDimensionsZ = sampleDimensionsY;
    }
  } else {
    sampleDimensionsY = sampleDimensionsX;
    sampleDimensionsZ = sampleDimensionsX;
  }
  return splat(argv[1], argv[2], standardDeviation,
    sampleDimensionsX, sampleDimensionsY, sampleDimensionsZ);
}
