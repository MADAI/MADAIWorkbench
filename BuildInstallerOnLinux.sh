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
# Convert relative paths to full paths
###################################
fullpath () (
    cd $1
    echo `pwd`
)

###################################
# Define important variables
###################################
madaiworkbench_build_dir=`fullpath $1`

###################################
# Find ParaView build directory
###################################
paraview_build_dir=`grep "ParaView_DIR:PATH" $madaiworkbench_build_dir/CMakeCache.txt | cut -d '=' -f 2`
echo "Found ParaView build directory $paraview_build_dir"

###################################
# Find QtSDK directory
###################################
qmake_path=`grep "QT_QMAKE_EXECUTABLE_LAST:INTERNAL" $madaiworkbench_build_dir/CMakeCache.txt | cut -d '=' -f 2`
qt_lib_dir=`dirname $qmake_path`
qt_lib_dir=`fullpath $qt_lib_dir/../lib`
echo "Found Qt binary directory $qt_lib_dir"

###################################
# Assemble the files into the
# desired directory structure.
###################################
install_dir=`pwd`/installer-working-directory
rm -rf $install_dir
bin_dir=$install_dir/bin
lib_dir=$install_dir/lib
mkdir -p $install_dir
mkdir -p $bin_dir
mkdir -p $lib_dir


cp -r "$paraview_build_dir/bin"/paraview $bin_dir
cp -r "$paraview_build_dir/bin"/pvbatch $bin_dir
cp -r "$paraview_build_dir/bin"/pvdataserver $bin_dir
cp -r "$paraview_build_dir/bin"/pvpython $bin_dir
cp -r "$paraview_build_dir/bin"/pvrenderserver $bin_dir
cp -r "$paraview_build_dir/bin"/pvserver $bin_dir
cp -r "$paraview_build_dir/bin"/*.so* $lib_dir
cp -r "$madaiworkbench_build_dir/bin/MADAIWorkbench" $bin_dir
cp -r "$madaiworkbench_build_dir/bin"/*.so $bin_dir
cp -r "$madaiworkbench_build_dir/bin"/.plugins $bin_dir
cp -r "$qt_lib_dir/"* $lib_dir

###################################
# Create a script to run the workbench
###################################
echo -e "#!/bin/bash\n\n"                         \
    "script_path=\`pwd\`/\$0\n"                   \
    "bin_dir=\`dirname \$script_path\`\n"         \
    "export PATH=\$bin_dir:\$bin_dir/../lib\n"    \
    "export LD_LIBRARY_PATH=\$bin_dir/../lib\n\n" \
    "\$bin_dir/MADAIWorkbench\n" > $bin_dir/RunMADAIWorkbench.sh
chmod 755 $bin_dir/RunMADAIWorkbench.sh

###################################
# Now tar up the file
###################################
pushd $install_dir
tar -cvzf ../MADAIWorkbench.tar.gz ./bin ./lib
popd

###################################
# Cleanup
###################################
rm -rf $install_dir
