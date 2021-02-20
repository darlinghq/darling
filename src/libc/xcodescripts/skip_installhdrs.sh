#!/bin/bash

if [[ "x${ACTION}" == "xinstallhdrs" ]]; then
	exit 0
fi

if [[ "x${ACTION}" == "xinstallapi" ]]; then
	exit 0
fi

$@
