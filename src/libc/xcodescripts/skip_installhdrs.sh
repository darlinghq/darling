#!/bin/bash

if [[ "x${ACTION}" == "xinstallhdrs" ]]; then
	exit 0
fi

$@
