#!/usr/bin/env bash

export DARLING_SRC=`pwd`
VERSION=1.0

# Test if darling-mach is already installed
modinfo darling-mach &> /dev/null

if (( $? == 0 )); then
  dkms remove darling-mach/$VERSION --all
fi

# Build it from scratch and install
dkms add .
dkms build darling-mach -v $VERSION
dkms install darling-mach -v $VERSION
