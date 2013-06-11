#!/bin/bash

###################################
# Options
###################################
PARAVIEW_GIT_URL="git://paraview.org/ParaView.git"

source SetParaViewVersion.sh

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
[ -x "${qmake}" ] || die "qmake: \"${qmake}\" not found."
mkdir -p $2
build_dir=`fullpath $2`
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

target="10.6"
if [ "$4" == "10.5" -o "$4" == "10.6" ]; then
    target="$4"
else
    echo "Invalid target $4. Must be 10.5 or 10.6."
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
mkdir -p ${build_dir}
bin_dir=${build_dir}/bin
mkdir -p ${bin_dir}
src_dir=${build_dir}/src
mkdir -p ${src_dir}

###################################
# Clone VRPN
###################################
cd ${src_dir}
vrpn_src_dir=${src_dir}/VRPN
if [ ! -d ${src_dir}/VRPN ]
    then git clone --recursive git://git.cs.unc.edu/vrpn.git VRPN || die "Could not clone VRPN"; git submodule update --init
fi
cd ${vrpn_src_dir}

###################################
# Configure VRPN
###################################
vrpn_build_dir=${bin_dir}/VRPN-build
mkdir -p ${vrpn_build_dir}
cd ${vrpn_build_dir}
cmake \
    -D CMAKE_BUILD_TYPE:STRING=${build_type} \
    -D BUILD_TESTING:BOOL=OFF \
    -D CMAKE_OSX_DEPLOYMENT_TARGET:STRING=${target} \
    -D CMAKE_OSX_SYSROOT:PATH=/Developer/SDKs/MacOSX${target}.sdk \
    -D VRPN_USE_HID:BOOL=ON \
    -D VRPN_USE_LOCAL_HIDAPI:BOOL=ON \
    ${vrpn_src_dir}
cmake .

# Build VRPN
make -j ${num_cores}

###################################
# Clone ParaView
###################################
cd ${src_dir}
paraview_src_dir=${src_dir}/ParaView
if [ ! -d ${src_dir}/ParaView ]
    then git clone ${PARAVIEW_GIT_URL} ParaView || die "Could not clone ParaView"
fi
cd ${paraview_src_dir}

# Switch to desired ParaView version
git fetch origin
git checkout ${PARAVIEW_COMMIT}
git submodule update --init

###################################
# Configure ParaView
###################################
paraview_build_dir=${bin_dir}/ParaView-build
mkdir -p ${paraview_build_dir}
cd ${paraview_build_dir}
cmake \
    -D CMAKE_BUILD_TYPE:STRING=${build_type} \
    -D BUILD_SHARED_LIBS:BOOL=ON \
    -D BUILD_TESTING:BOOL=OFF \
    -D PARAVIEW_ENABLE_PYTHON:BOOL=ON \
    -D PARAVIEW_ENABLE_PYTHON_FILTERS:BOOL=ON \
    -D PYTHON_EXECUTABLE:PATH=/usr/bin/python${python_version} \
    -D PYTHON_INCLUDE_DIR:PATH=/System/Library/Frameworks/Python.framework/Versions/${python_version}/Headers \
    -D PYTHON_LIBRARY:PATH=/usr/lib/libpython${python_version}.dylib \
    -D PARAVIEW_USE_VISITBRIDGE:BOOL=OFF \
    -D PARAVIEW_BUILD_PLUGIN_VRPlugin:BOOL=ON \
    -D PARAVIEW_USE_VRPN:BOOL=ON \
    -D PARAVIEW_USE_VRUI:BOOL=OFF \
    -D VRPN_INCLUDE_DIR:PATH=${vrpn_src_dir} \
    -D VRPN_LIBRARY:FILEPATH=${vrpn_build_dir}/libvrpn.a \
    -D QT_QMAKE_EXECUTABLE:PATH=${qmake} \
    -D CMAKE_OSX_DEPLOYMENT_TARGET:STRING=${target} \
    -D CMAKE_OSX_SYSROOT:PATH=/Developer/SDKs/MacOSX${target}.sdk \
    ${paraview_src_dir}
cmake .

# Build ParaView
make -j ${num_cores}

###################################
# Configure MADAIWorkbench
###################################
madaiworkbench_build_dir=${bin_dir}/MADAIWorkbench-build
mkdir -p ${madaiworkbench_build_dir}
cd ${madaiworkbench_build_dir}
cmake \
    -D CMAKE_BUILD_TYPE:STRING=${build_type} \
    -D BUILD_APPLICATION:BOOL=ON \
    -D BUILD_SHARED_LIBS:BOOL=ON \
    -D ParaView_DIR:PATH=${paraview_build_dir} \
    -D CMAKE_OSX_DEPLOYMENT_TARGET:STRING=${target} \
    -D CMAKE_OSX_SYSROOT:PATH=/Developer/SDKs/MacOSX${target}.sdk \
    ${madaiworkbench_src_dir}
cmake .

# Build MADAIWorkbench
make -j ${num_cores}

###################################
# Make the installer
###################################
cpack -G DragNDrop --config ${madaiworkbench_build_dir}/Application/CPackMADAIWorkbenchConfig.cmake
