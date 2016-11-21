#!/usr/bin/env bash
dkms remove -m darling-mach -v 1.0 --all
rm -rf /var/lib/dkms/darling-mach
rm -rf /usr/src/darling-mach-1.0

