#!/bin/bash

###################################
# Argument checking
###################################
die () {
    echo >&2 "$@"
    exit 1
}

[ "$#" -gt 1 ] || die "Usage: BuildInstallerOnMacOSX.sh <path to qmake> <build directory> [Release | Debug]"

###################################
# Convert relative paths to full paths
###################################
fullpath () (
    cd $1
    echo `pwd`
)

###################################
# Define important variables
###################################
qmake=$1
build_dir=$2
script_relative_path=`dirname $0`
script_dir=`fullpath $script_relative_path`
madaiworkbench_src_dir=$script_dir
num_cores=`sysctl -n hw.ncpu`

build_type=Release
if [ "$3" == Debug ]; then
    build_type=Debug
fi
echo "Build type:" ${build_type}

###################################
# Set up build and source directories
###################################
mkdir -p $build_dir
bin_dir=$build_dir/bin
mkdir -p $bin_dir
src_dir=$build_dir/src
mkdir -p $src_dir

###################################
# Clone ParaView
###################################
cd $src_dir
paraview_src_dir=$src_dir/ParaView
git clone --recursive git://paraview.org/ParaView.git ParaView
cd $paraview_src_dir

# Switch to tag v3.12.0
git checkout v3.12.0
git submodule update

###################################
# Configure ParaView
###################################
paraview_build_dir=$bin_dir/ParaView-build
mkdir -p $paraview_build_dir
cd $paraview_build_dir
cmake \
    -D CMAKE_BUILD_TYPE:STRING=${build_type} \
    -D BUILD_SHARED_LIBS:BOOL=ON \
    -D BUILD_TESTING:BOOL=OFF \
    -D PARAVIEW_ENABLE_PYTHON:BOOL=ON \
    -D PARAVIEW_ENABLE_PYTHON_FILTERS:BOOL=ON \
    -D PYTHON_EXECUTABLE:PATH=/usr/bin/python2.5 \
    -D PYTHON_INCLUDE_DIR:PATH=/System/Library/Frameworks/Python.framework/Versions/2.5/Headers \
    -D PYTHON_LIBRARY:PATH=/usr/lib/libpython2.5.dylib \
    -D PARAVIEW_USE_VISITBRIDGE:BOOL=ON \
    -D QT_QMAKE_EXECUTABLE:PATH=$qmake \
    -D CMAKE_OSX_DEPLOYMENT_TARGET:STRING=10.5 \
    -D CMAKE_OSX_SYSROOT:PATH=/Developer/SDKs/MacOSX10.5.sdk \
    $paraview_src_dir
cmake .

# Build ParaView
make -j $num_cores

###################################
# Configure MADAIWorkbench
###################################
madaiworkbench_build_dir=$bin_dir/MADAIWorkbench-build
mkdir -p $madaiworkbench_build_dir
cd $madaiworkbench_build_dir
cmake \
    -D CMAKE_BUILD_TYPE:STRING=${build_type} \
    -D BUILD_APPLICATION:BOOL=ON \
    -D BUILD_SHARED_LIBS:BOOL=ON \
    -D ParaView_DIR:PATH=$paraview_build_dir \
    -D CMAKE_OSX_DEPLOYMENT_TARGET:STRING=10.5 \
    -D CMAKE_OSX_SYSROOT:PATH=/Developer/SDKs/MacOSX10.5.sdk \
    $madaiworkbench_src_dir
cmake .

# Build MADAIWorkbench
make -j $num_cores

###################################
# Make the installer
###################################
cpack -G DragNDrop --config $madaiworkbench_build_dir/Application/CPackMADAIWorkbenchConfig.cmake
