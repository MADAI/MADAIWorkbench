#!/bin/sh
[ $(id -u) = 0 ] || exec sudo "$0" "$@"
#Author: Cory Quammen <cquammen@cs.unc.edu>
#Last updated: June 10, 2013

# WARNING: You must run this script with sudo
#
# This script installs prerequisites necessary to build the MADAI
# Workbench on Ubuntu. It was developed and tested under Ubuntu 12.04,
# but should work with other versions.

apt-get install -y   \
    build-essential  \
    ia32-libs        \
    git              \
    libusb-1.0.0-dev \
    python2.7-dev    \
    libqt4-dev       \
    qt4-dev-tools    \
    libxt-dev        \
    libboost-dev

# Install CMake 2.8.10 or higher.

