This is a rework of Darling, which will greatly improve compatibility with OS X applications.

The i386 build is still lagging behind due to some missing important assembly around threading. You only need it to run 32-bit binaries though.

Courageous users may proceed with the build instructions below.

For x86-64 OS X binaries:
````
cd darling
mkdir -p build/x86-64
CC=clang CXX=clang++ cmake ../.. -DCMAKE_INSTALL_LIBDIR=lib64
make
make install

# Now we go into src/lkm to build the kernel module
cd ../../src/lkm
make
make install
````

For i386 OS X binaries:
````
cd darling
mkdir -p build/i386
CC=clang CXX=clang++ cmake ../.. -DCMAKE_INSTALL_LIBDIR=lib32
make
make install
````

Loading the kernel module:
````
modprobe darling-mach

# ATTENTION: The kernel module is likely unstable,
# full of vulnerabilities, etc.
# You SHOULD restrict access to /dev/mach to trusted
# users only and be prepared to the eventuality of
# kernel hangups (and related data loss).

chmod a+rw /dev/mach
````

