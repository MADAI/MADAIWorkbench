#!/bin/bash

###################################
# Options
###################################
PARAVIEW_VERSION="1439967260d37255e86d328424597a6e93921a3f"
#PARAVIEW_VERSION="v3.14.1"
#PARAVIEW_VERSION="MADAIWorkbench"

PARAVIEW_GIT_URL="git://paraview.org/ParaView.git"
#PARAVIEW_GIT_URL="git://github.com/MADAI/ParaView.git"


###################################
# Argument checking
###################################
die () {
    echo >&2 "$@"
    exit 1
}

[ "$#" -gt 1 ] || die "Usage: BuildInstallerOnMacOSX.sh <path to qmake> <build directory> [Release | Debug] [10.5 | 10.6]"

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
[ -x "$qmake" ] || die "qmake: \"${qmake}\" not found."
build_dir=$2
script_relative_path=`dirname $0`
script_dir=`fullpath $script_relative_path`
madaiworkbench_src_dir=$script_dir
num_cores=`sysctl -n hw.ncpu`

###################################
# Determine the build target
# This is the version of OS X
# on which you want the Workbench
# to run.
###################################
build_type=Release
if [ "$3" == Debug ]; then
    build_type=Debug
fi
echo "Build type:" ${build_type}

target=10.6
if [ "$4" == "10.5" -o "$4" == "10.6" ]; then
    target="$4"
else
    echo "Invalid target ${target}. Must be 10.5 or 10.6."
    exit
fi
echo "Target: " ${target}

# On Mac OS X 10.5, the install Python is version 2.5
python_version="2.6"
if [ "$4" == "10.5" ]; then
    python_version="2.5"
fi

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
git clone ${PARAVIEW_GIT_URL} ParaView
cd $paraview_src_dir

# Switch to desired ParaView version
git fetch origin
git checkout ${PARAVIEW_VERSION}
git submodule update --init

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
    -D PYTHON_EXECUTABLE:PATH=/usr/bin/python${python_version} \
    -D PYTHON_INCLUDE_DIR:PATH=/System/Library/Frameworks/Python.framework/Versions/${python_version}/Headers \
    -D PYTHON_LIBRARY:PATH=/usr/lib/libpython${python_version}.dylib \
    -D PARAVIEW_USE_VISITBRIDGE:BOOL=ON \
    -D QT_QMAKE_EXECUTABLE:PATH=$qmake \
    -D CMAKE_OSX_DEPLOYMENT_TARGET:STRING=${target} \
    -D CMAKE_OSX_SYSROOT:PATH=/Developer/SDKs/MacOSX${target}.sdk \
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
    -D CMAKE_OSX_DEPLOYMENT_TARGET:STRING=${target} \
    -D CMAKE_OSX_SYSROOT:PATH=/Developer/SDKs/MacOSX${target}.sdk \
    $madaiworkbench_src_dir
cmake .

# Build MADAIWorkbench
make -j $num_cores

###################################
# Make the installer
###################################
cpack -G DragNDrop --config $madaiworkbench_build_dir/Application/CPackMADAIWorkbenchConfig.cmake
