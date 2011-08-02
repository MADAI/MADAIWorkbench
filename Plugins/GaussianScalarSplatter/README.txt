
Program:  Paraview / Visualization Toolkit - Gaussian Scalar Splat
Modules:  GaussianScalarSplatter.xml / vtkGaussianScalarSplatter.cxx

  This is a VTK filter class as well as a ParaView Plugin that makes
  use of the filter.  The filter can be applied to any vtkDataSet that
  has PointData, but is intended for use on vtkPolyData that contains
  a set of points.

  For each vtkDataArray attached to the points, this filter will
  Gaussian splat those values into a vtkImageData grid.  The only
  user-set parameter is StandardDeviation, the standard deviation of
  the Gaussians used for the splat.

  The output will have one aditional vtkDataArray called NumberDensity
  which simply gives the number of points per unit volume.

 Written 2011 Hal Canary <hal@cs.unc.edu> for the MADAI project
 <http://MADAI.us>.  Based on vtkGaussianSplatter written by Ken
 Martin, Will Schroeder, Bill Lorensen.

  See http://www.kitware.com/Copyright.htm for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A
  PARTICULAR PURPOSE.  See the above copyright notice for more
  information.
