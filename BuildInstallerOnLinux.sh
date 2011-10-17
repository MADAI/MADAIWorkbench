#!/bin/bash

###################################
# Argument checking
###################################
die () {
    echo >&2 "$@"
    exit 1
}

[ "$#" -eq 1 ] || die "Usage: BuildInstallerOnLinux.sh <MADAIWorkbench build directory>"

###################################
# Define important variables
###################################
installer_script_dir=`dirname $0`
installer_script_dir=`readlink -f $installer_script_dir`
madaiworkbench_build_dir=`readlink -f $1`

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
qmake_path=`grep "QT_QMAKE_EXECUTABLE_LAST:INTERNAL" ${madaiworkbench_build_dir}/CMakeCache.txt | cut -d '=' -f 2`
qt_lib_dir=`dirname ${qmake_path}`
qt_lib_dir=`readlink -f ${qt_lib_dir}/../lib`
echo "Found Qt binary directory ${qt_lib_dir}"

###################################
# Find MPI binaries
# (assumes MPICH-2 is used)
###################################
mpi_include_dir=`grep "MPI_INCLUDE_PATH:" ${paraview_build_dir}/CMakeCache.txt | cut -d '=' -f 2`
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
cp -r ${qt_lib_dir}/*                 ${lib_dir}

# Copy MPI items
mpi_items=(bt2line check_callstack clog2_join clog2_print clog2_repair \
    hydra_nameserver hydra_persist hydra_pmi_proxy mpic++ mpicc \
    mpich2version mpicxx mpiexec mpiexec.hydra mpirun)
for item in ${mpi_items[*]}
do
    echo "Copying MPI binary ${item}"
    cp -r ${mpi_bin_dir}/${item} ${lib_dir}
done

###################################
# Configure scripts to run the
# workbench and server
###################################
script_name=$bin_dir/../MADAIWorkbench
cp ${installer_script_dir}/scripts/MADAIWorkbench.in ${script_name}
sed -i 's/@VERSION@/'${version}'/g' ${script_name}
chmod 755 $script_name
script_name=$bin_dir/../MADAIWorkbenchServer
cp ${installer_script_dir}/scripts/MADAIWorkbenchServer.in ${script_name}
sed -i 's/@VERSION@/'${version}'/g' ${script_name}
chmod 755 $script_name

###################################
# Now tar up the file
###################################
pushd ${install_dir}
tar -czf ../MADAIWorkbench-${version}.tar.gz ./bin ./lib
popd

###################################
# Cleanup
###################################
rm -rf ${install_dir}
