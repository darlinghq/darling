# Sed script for tests/stateful-check editing.

/set -e/ a\
# For systems with severe filename restrictions allow for\
# an alternate filename.\
UNAME=${UNAME-`uname 2>/dev/null`}\
case X$UNAME in\
  *-DOS) filename=`echo "$charset" | sed "s|ISO-|ISO/|;s|2022-|2022|"` ;;\
  *)     filename="$charset" ;;\
esac
s/\$charset"-snippet/$filename"-snippet/g
