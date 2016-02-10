set -e -x

BINDIR="$DSTROOT/$DT_TOOLCHAIN_DIR/usr/bin"

install -d -o root -g wheel -m 0755 "$BINDIR"
install -c -o root -g wheel -m 0755 \
	"$PROJECT_DIR/migcom.tproj/mig.sh" \
	"$BINDIR"/mig
