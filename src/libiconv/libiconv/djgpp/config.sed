# Additional editing of Makefiles
/(echo[ 	]*':t/ a\
# DJGPP specific Makefile changes.\
  /^aliaspath *	*=/s,:,";",g;t t\
  /TEXINPUTS=/s,:,";",g;t t\
  /PATH=/s,:,";",g;t t\
  s,\\.deps,_deps,g;t t\
  s,\\.libs,_libs,g;t t\
  s,\\.new\\.,_new.,g;t t\
  s,\\.old\\.,_old.,g;t t\
  s,\\.tab\\.,_tab.,g;t t\
  /^lispdir *	*=/ c\\\
lispdir = \$(prefix)/gnu/emacs/site-lisp\
  /^docdir *	*=/ c\\\
docdir = \$(prefix)/gnudocs/libiconv.110\
  /(datadir)\\/doc/d\
  s,config\\.h\\.in,config.h-in,g;t t\
  s,\\.\\([1-9]\\)\\.html,_\\1.html,g;t t\
  s,/doc,/gnudocs,g;t t\
  s,iconv\\.h\\.in,iconv.h-in,g;t t\
  s,libcharset\\.h\\.in,libcharset.h-in,g;t t\
  s,localcharset\\.h\\.in,localcharset.h-in,g;t t\
  s,Makefile\\.in\\.in,Makefile.in-in,g;t t\
  s,Makefile\\.am\\.in,Makefile.am-in,g;t t\
  /^install-info-am:/,/^$/ s,file-\\[0-9\\]\\[0-9\\],& \\$\\$file[0-9] \\$\\$file[0-9][0-9],\
  /^\\.y\\.c:/,/^$/ {\
    /\\\$(YACC)/ {\
      s,[ 	]*&&.*$,,\
      a\\\
	-mv -f y_tab.c \$*.c\\\
	-@test -f y.tab.c && mv -f y.tab.c \$*.c\\\
	-@test -f y.tab.h && mv -f y.tab.h y_tab.h\
    }\
  }

# Rename config.h.in into config.h-in
/^[ 	]*ac_config_headers=/,/^_ACEOF/ {
  s|config\.h|&:config.h-in|g
}
/CONFIG_HEADERS=/ s|config\.h\.in|&:config.h-in|2


# Rename iconv.h.in into iconv.h-in
/^[ 	]*ac_config_files=/,/^EOF/ {
  s|include/iconv\.h|&:include/iconv.h-in|
  s|po/Makefile\.in|&:po/Makefile.in-in|
}
/CONFIG_FILES=/ {
  s|include/iconv\.h|&:include/iconv\.h-in|2
  s|po/Makefile\.in|&:po/Makefile.in-in|2
}

# We always use _deps and _libs instead of .deps and .libs, because
# the latter is an invalid name on 8+3 MS-DOS filesystem.  This makes
# the generated Makefiles good for every DJGPP installation, not only
# the one where the package was configured (which could happen to be
# a Windows box, where leading dots in file names are allowed).
s,\.deps,_deps,g
s,\.libs,_libs,g
/^rmdir[ 	]*\.tst/ i\
am__leading_dot=_

# Replace (command) > /dev/null with `command > /dev/null`, since
# parenthesized commands always return zero status in the ported Bash,
# even if the named command doesn't exist
/if ([^|;`]*null/{
  s,(,`,
  s,),,
  /null[ 	]*2>&1/ s,2>&1,&`,
  /null.*null/ s,null.*null,&`,
  /null.*null/ !{
    /null[ 	]*2>&1/ !s,null,&`,
  }
}

# DOS-style absolute file names should be supported as well
/\*) srcdir=/s,/\*,[\\\\/]* | [A-z]:[\\\\/]*,
/\$]\*) INSTALL=/s,\[/\$\]\*,[\\\\/$]* | [A-z]:[\\\\/]*,
/\$]\*) ac_rel_source=/s,\[/\$\]\*,[\\\\/$]* | [A-z]:[\\\\/]*,

# Switch the order of the two Sed commands, since DOS path names
# could include a colon
/ac_file_inputs=/s,\( -e "s%\^%\$ac_given_srcdir/%"\)\( -e "s%:% $ac_given_srcdir/%g"\),\2\1,

# Prevent the spliting of conftest.subs.
# The sed script: conftest.subs is split into 48 or 90 lines long files.
# This will produce sed scripts called conftest.s1, conftest.s2, etc.
# that will not work if conftest.subs contains a multi line sed command
# at line #90. In this case the first part of the sed command will be the
# last line of conftest.s1 and the rest of the command will be the first lines
# of conftest.s2. So both script will not work properly.
# This matches the configure script produced by Autoconf 2.57
/ac_max_sed_lines=[0-9]/ s,=.*$,=`sed -n "$=" $tmp/subs.sed`,

# The following two items are changes needed for configuring
# and compiling across partitions.
# 1) The given srcdir value is always translated from the
#    "x:" syntax into "/dev/x" syntax while we run configure.
/^[ 	]*-srcdir=\*.*$/ a\
    ac_optarg=`echo "$ac_optarg" | sed "s,^\\([A-Za-z]\\):,/dev/\\1,"`
/set X `ls -Lt \$srcdir/ i\
   if `echo $srcdir | grep "^/dev/" - > /dev/null`; then\
     srcdir=`echo "$srcdir" | sed -e "s%^/dev/%%" -e "s%/%:/%"`\
   fi

#  2) We need links across partitions,
#     so we will use "cp -pf" instead of "ln".
/# Make a symlink if possible; otherwise try a hard link./,/EOF/ {
  s,;.*then, 2>/dev/null || cp -pf \$srcdir/\$ac_source \$ac_dest&,
}

# Autoconf 2.52e generated configure scripts
# write absolute paths into Makefiles making
# them useless for DJGPP installations for which
# the package has not been configured for.
/MISSING=/,/^$/ {
  /^fi$/ a\
am_missing_run=`echo "$am_missing_run" | sed 's%/dev.*/libiconv-[0-9]\\{1,1\\}[-.0-9]*%${top_srcdir}%'`
}
/^install_sh=/a\
install_sh=`echo "$install_sh" | sed 's%/dev.*/libiconv-[0-9]\\{1,1\\}[-.0-9]*%${top_srcdir}%'`

# The following makes sure we are not going to remove a directory
# which is the cwd on its drive (DOS doesn't allow to remove such
# a directory).  The trick is to chdir to the root directory on
# temp directory's drive before removing $tmp.
/^[ 	]*trap[ 	]*'exit_status=\$\?;[ 	]*rm[ 	]*-rf/ s%rm -rf%cd $tmp; cd /; &%

# AC_CONFIG_LINKS fails if the source and destination are on
# different file systems and symlinks don't work.
/^    ln \$srcdir/s%||%|| cp -pf $srcdir/$ac_source $ac_dest ||%

# Add DJGPP version information.
/^[ 	]*VERSION=/ s/\([0-9]\.[0-9]*[-.0-9]*\)/"\1  (DJGPP port 2006-03-21 (r1))"/

# We need makeinfo to make the html formated docs.
/\$am_missing_run[ 	]*makeinfo/ s,\$am_missing_run,,

# The path to the FORTRAN compiler and libraries
# shall contain no absolute path reference so it
# will be good for all djgpp installations.
/^FLIBS="\$ac_cv_flibs"/ i\
ac_djgpp_path=`echo "$DJDIR" | sed 's%\\\\\\%/%g' | tr $as_cr_LETTERS $as_cr_letters`\
ac_cv_flibs=`echo "$ac_cv_flibs" | sed "s%-L$ac_djgpp_path%-L/dev/env/DJDIR%g"`

# Let libtool use _libs all the time.
/objdir=/s,\.libs,_libs,

# DJGPP needs ICONV_CONST defined as "const"
/if .*am_cv_proto_iconv_arg1/ i\
am_cv_proto_iconv_arg1="const"

# DJGPP installs the man pages into ${prefix}/man and not into ${datadir}/man
/^test .*{datadir}\/man/ d
