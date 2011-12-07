#!/bin/bash

###################################
# Argument checking
###################################
die () {
    echo >&2 "$@"
    exit 1
}

[ "$#" -eq 2 ] || die "Usage: BuildInstallerOnLinux.sh <path to qmake> <build directory>"

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
current_dir=`pwd`
qmake=$1
build_dir=$2
script_relative_path=`dirname $0`
script_dir=`fullpath $script_relative_path`
madaiworkbench_src_dir=$script_dir
num_cores=`cat /proc/cpuinfo | grep "core id" | wc -l`

###################################
# Set up build and source directories
###################################
mkdir -p $build_dir
bin_dir=$build_dir/bin
mkdir -p $bin_dir
src_dir=$build_dir/src
mkdir -p $src_dir
install_dir=$build_dir/install
mkdir -p $install_dir

###################################
# Get MPI
###################################
cd $src_dir
mpich2_src_dir=$src_dir/mpich2-1.4
wget --timestamping http://www.mcs.anl.gov/research/projects/mpich2/downloads/tarballs/1.4/mpich2-1.4.tar.gz
tar xzf mpich2-1.4.tar.gz

export CFLAGS=-fPIC
export CXXFLAGS=-fPIC

###################################
# Configure MPICH2
###################################
pushd mpich2-1.4
if [ ! -f config.log ]; then
    ./configure --prefix=$install_dir --enable-threads=runtime --disable-f77 --disable-fc
fi
popd

###################################
# Build MPI
###################################
nice make -j $num_cores
nice make install

unset CLFAGS
unset CXXFLAGS

###################################
# All following libraries require MPI-enabled compilers
###################################
export CC=$install_dir/bin/mpicc
export CXX=$install_dir/bin/mpicxx

###################################
# Clone ParaView
###################################
cd $src_dir
paraview_src_dir=$src_dir/ParaView
git clone --recursive git://paraview.org/ParaView.git ParaView
cd $paraview_src_dir
git checkout v3.12.0

###################################
# Configure ParaView
###################################
paraview_build_dir=$bin_dir/ParaView-build
mkdir -p $paraview_build_dir
cd $paraview_build_dir
cmake \
    -D CMAKE_BUILD_TYPE:STRING=Release \
    -D BUILD_SHARED_LIBS:BOOL=ON \
    -D BUILD_TESTING:BOOL=OFF \
    -D PARAVIEW_USE_MPI:BOOL=ON \
    -D MPI_CC_COMPILER:PATH=$CC \
    -D MPI_CXX_COMPILER:PATH=$CXX \
    -D PARAVIEW_ENABLE_PYTHON:BOOL=ON \
    -D PARAVIEW_ENABLE_PYTHON_FILTERS:BOOL=ON \
    -D PYTHON_EXECUTABLE:PATH=/usr/bin/python \
    -D PYTHON_INCLUDE_DIR:PATH=/usr/include/python2.6 \
    -D PYTHON_LIBRARY:PATH=/usr/lib64/libpython2.6.so \
    -D PARAVIEW_USE_VISITBRIDGE:BOOL=ON \
    -D QT_QMAKE_EXECUTABLE:PATH=$qmake \
    $paraview_src_dir
cmake .

###################################
# Build ParaView
###################################
nice make -j $num_cores

###################################
# Configure MADAIWorkbench
###################################
madaiworkbench_build_dir=$bin_dir/MADAIWorkbench-build
mkdir -p $madaiworkbench_build_dir
cd $madaiworkbench_build_dir
cmake \
    -D CMAKE_BUILD_TYPE:STRING=Release \
    -D BUILD_APPLICATION:BOOL=ON \
    -D BUILD_SHARED_LIBS:BOOL=ON \
    -D ParaView_DIR:PATH=$paraview_build_dir \
    $madaiworkbench_src_dir
cmake .

###################################
# Build MADAIWorkbench
###################################
nice make -j $num_cores

###################################
# Get version string
###################################
version=`grep "SET(CPACK_PACKAGE_VERSION " ${madaiworkbench_build_dir}/CPackSourceConfig.cmake | cut -d '"' -f 2`
echo "Creating installer for MADAI Workbench ${version}"

###################################
# Find ParaView build directory
###################################
paraview_build_dir=`grep "ParaView_DIR:PATH" ${madaiworkbench_build_dir}/CMakeCache.txt | cut -d '=' -f 2`
echo "Found ParaView build directory ${paraview_build_dir}"

###################################
# Find QtSDK directory
###################################
qmake_path=`grep "QT_QTCORE_LIBRARY_RELEASE:" ${madaiworkbench_build_dir}/CMakeCache.txt | cut -d '=' -f 2`
echo ${qmake_path}
qt_lib_dir=`dirname ${qmake_path}`
echo "Found Qt binary directory ${qt_lib_dir}"

###################################
# Find MPI binaries
# (assumes MPICH-2 is used)
###################################
#mpi_include_dir=`grep "MPI_INCLUDE_PATH:" ${paraview_build_dir}/CMakeCache.txt | cut -d '=' -f 2`
mpi_include_dir=$install_dir/include
mpi_bin_dir=`dirname ${mpi_include_dir}`/bin
echo "Found MPI binary directory ${mpi_bin_dir}"

###################################
# Assemble the files into the
# desired directory structure.
###################################
install_dir=`pwd`/installer-working-directory
rm -rf ${install_dir}
bin_dir=${install_dir}/bin/MADAIWorkbench-${version}
plugin_dir=${bin_dir}/plugins
lib_dir=${install_dir}/lib/MADAIWorkbench-${version}
mkdir -p ${install_dir}
mkdir -p ${bin_dir}
mkdir -p ${plugin_dir}
mkdir -p ${lib_dir}

# Copy ParaView binaries
pv_binaries=(paraview pvbatch pvdataserver pvpython pvrenderserver \
    pvserver)
for item in ${pv_binaries[*]}
do
    echo "Copying ParaView binary ${item}"
    cp -r ${paraview_build_dir}/bin/${item} ${bin_dir}
done

# Copy ParaView libraries
cp -r ${paraview_build_dir}/bin/*.so*             ${lib_dir}

# Copy MADAIWorkbench binaries
cp -r ${madaiworkbench_build_dir}/bin/MADAIWorkbench ${bin_dir}

# Get list of plugins
plugin_list=`grep PARAVIEW_PLUGINLIST ${madaiworkbench_build_dir}/CMakeCache.txt | cut -d'=' -f 2 | sed 's/;/\n/g' | sort | uniq`

pushd ${plugin_dir}
for plugin in ${plugin_list}
do
    echo "Copying plugin ${plugin}"
    plugin_lib=lib${plugin}.so
    cp ${madaiworkbench_build_dir}/bin/${plugin_lib} ${bin_dir}
    ln -s ../${plugin_lib}
done
popd

# Copy Qt libraries
cp -r -L ${qt_lib_dir}/libQt*.so ${lib_dir}

# Copy MPI items
mpi_items=(bt2line check_callstack clog2_join clog2_print clog2_repair \
    hydra_nameserver hydra_persist hydra_pmi_proxy mpic++ mpicc \
    mpich2version mpicxx mpiexec mpiexec.hydra mpirun)
for item in ${mpi_items[*]}
do
    echo "Copying MPI binary ${item}"
    cp -r ${mpi_bin_dir}/${item} ${lib_dir}
done

####################################
# Configure scripts to run the
# workbench and server
###################################
script_name=$bin_dir/../MADAIWorkbench
cp ${script_dir}/scripts/MADAIWorkbench.in ${script_name}
sed -i 's/@VERSION@/'${version}'/g' ${script_name}
chmod 755 $script_name
script_name=$bin_dir/../MADAIWorkbenchServer
cp ${script_dir}/scripts/MADAIWorkbenchServer.in ${script_name}
sed -i 's/@VERSION@/'${version}'/g' ${script_name}
chmod 755 $script_name

###################################
# Now tar up the file
###################################
pushd ${install_dir}
tar -czf ${current_dir}/MADAIWorkbench-${version}.tar.gz ./bin ./lib
popd

###################################
# Cleanup
###################################
rm -rf ${install_dir}