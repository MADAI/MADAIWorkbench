
                                * * *

Program:   Visualization Toolkit & ParaView - Binning Filter
Module:    vtkBinningFilter.cxx / BinningFilter.xml

Splat points into a volume by distributing them into bins and summing
up the data arrays in each bin.

                                * * *
 
  Written 2011 Hal Canary <hal@cs.unc.edu> for the MADAI project
  <http://MADAI.us>.  Based on other VTK Filters, Copyright (c) Ken
  Martin, Will Schroeder, Bill Lorensen.  All rights reserved.
 
  See VTK_Copyright.txt or http://www.kitware.com/Copyright.htm for details.
 
    This software is distributed WITHOUT ANY WARRANTY; without even
    the implied warranty of MERCHANTABILITY or FITNESS FOR A
    PARTICULAR PURPOSE.  See the above copyright notice for more
    information.

                                * * *

vtkBinningFilter is a filter that injects values from input points
into a structured points (volume) dataset. As each point is injected,
it contributes to a single voxel, or "bin."

This class is typically used to convert point-valued distributions
into a volume representation. The volume is then usually iso-surfaced
or volume rendered to generate a visualization. It can be used to
create surfaces from point distributions, or to create structure
(i.e., topology) when none exists.

The input to this filter is any dataset type. This filter can be used
to resample any form of data, i.e., the input data need not be
unstructured.

Some voxels may never receive a contribution during the splatting
process.  The final value of these points is 0.

                                * * *
