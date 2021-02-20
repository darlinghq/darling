# Sed script for tests/stateless-check editing.
/set -e/ a\
# For systems with severe filename restrictions\
# allow for an alternate filename.\
UNAME=${UNAME-`uname 2>/dev/null`}\
case X$UNAME in\
  *-DOS) filename=`echo "$charset" | sed "s|ISO-|ISO/|; \\\
                                          s|Mac|Mac/|; \\\
                                          s|Georgian-|Georgian/|"`\
         tmp_filename=`echo "$filename" | sed "s|/|/tmp-|"`\
         tmp_orig_filename=`echo "$filename" | sed "s|/|/tmp-orig-|"` ;;\
  *)     filename="$charset"\
         tmp_filename="$charset"\
         tmp_orig_filename="$charset" ;;\
esac
s|/"\$charset"|/"$filename"|g
s|tmp-"\$charset"|"${srcdir}"/"$tmp_filename"|g
s|tmp-orig"\$charset"|"${srcdir}"/"$tmp_orig_filename"|g
s|\.INVERSE\.|.INVERSE-|g
s|\.IRREVERSIBLE\.|.IRREVERSIBLE-|g
