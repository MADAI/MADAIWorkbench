# ParaView / Visualization Toolkit - Gaussian Scalar Splat
#   Written 2011 Hal Canary <hal@cs.unc.edu> for the MADAI project
#   <http://MADAI.us>.  Based on vtkGaussianSplatter, Copyright (c) Ken
#   Martin, Will Schroeder, Bill Lorensen.  All rights reserved.
# See http://www.kitware.com/Copyright.htm for details.
#   This software is distributed WITHOUT ANY WARRANTY; without even
#   the implied warranty of MERCHANTABILITY or FITNESS FOR A
#   PARTICULAR PURPOSE.  See the above copyright notice for more
#   information.

add_paraview_plugin(
  GaussianScalarSplatter "1.0"
  SERVER_MANAGER_XML GaussianScalarSplatter.xml
  SERVER_MANAGER_SOURCES vtkGaussianScalarSplatter.cxx
  GUI_RESOURCE_FILES GaussianScalarSplatterGUI.xml
  )
