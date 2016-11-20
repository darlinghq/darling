#!/usr/bin/env bash
export DARLING_SRC=`pwd`
dkms add .
dkms build -m darling-mach -v 1.0
dkms install -m darling-mach -v 1.0

