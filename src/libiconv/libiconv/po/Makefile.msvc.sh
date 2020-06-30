#!/bin/sh
# Creates Makefile.msvc.
domain=$1
catalogs=$2

cat <<\EOF
# -*- Makefile -*- for po subdirectory

#### Start of system configuration section. ####

# Flags that can be set on the nmake command line:
#   PREFIX=Some\Directory   Base directory for installation
!if !defined(PREFIX)
PREFIX = c:\usr
!endif

# Directories used by "make":
srcdir = .

# Directories used by "make install":
prefix = $(PREFIX)
datadir = $(prefix)\share
localedir = $(datadir)\locale

# Programs used by "make":
RM = -del

# Programs used by "make install":
INSTALL = copy
INSTALL_PROGRAM = copy
INSTALL_DATA = copy

#### End of system configuration section. ####

SHELL = /bin/sh

all :

install : all force
	-mkdir $(prefix)
	-mkdir $(datadir)
	-mkdir $(localedir)
EOF
for cat in $catalogs; do
  cat=`basename $cat`
  lang=`echo $cat | sed -e 's/\.gmo$//'`
cat <<EOF
	-mkdir \$(localedir)\\${lang}
	-mkdir \$(localedir)\\${lang}\\LC_MESSAGES
	\$(INSTALL_DATA) ${lang}.gmo \$(localedir)\\${lang}\\LC_MESSAGES\\${domain}.mo
EOF
done
cat <<\EOF

installdirs : force
	-mkdir $(prefix)
	-mkdir $(datadir)
	-mkdir $(localedir)
EOF
for cat in $catalogs; do
  cat=`basename $cat`
  lang=`echo $cat | sed -e 's/\.gmo$//'`
cat <<EOF
	-mkdir \$(localedir)\\${lang}
	-mkdir \$(localedir)\\${lang}\\LC_MESSAGES
EOF
done
cat <<\EOF

uninstall : force
EOF
for cat in $catalogs; do
  cat=`basename $cat`
  lang=`echo $cat | sed -e 's/\.gmo$//'`
cat <<EOF
	\$(RM) \$(localedir)\\${lang}\\LC_MESSAGES\\${domain}.mo
EOF
done
cat <<\EOF

check : all

mostlyclean : clean

clean : force

distclean : clean

maintainer-clean : distclean

force :
EOF
