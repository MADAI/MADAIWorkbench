#!/bin/sh
[ $(id -u) = 0 ] || exec sudo "$0" "$@"

#Author: Cory Quammen <cquammen@cs.unc.edu>
#Last updated: July 18, 2013

# WARNING: You must run this script with sudo
#
# This script installs prerequisites necessary to build the MADAI
# Workbench on Ubuntu. It was developed and tested under Fedora 19
# but should work with other versions.

yum install -y     \
    git            \
    wget           \
    libusb1-devel  \
    gcc-c++        \
    python-devel   \
    qt-devel       \
    qtwebkit-devel \
    libXt-devel    \
    boost-devel    \
    cmake-gui

# I had to manually set some variables in the VRPN build directory
#
# LIBUSB1_INCLUDE_DIR /usr/include/libusb-1.0
# LIBUSB1_LIBRARY     /usr/lib64/libusb-1.0.so.0
# LIBUSB1_ROOT_DIR    /usr/lib64
