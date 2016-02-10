#!/bin/sh
set -e -x

# don't install man pages for installhdrs or iOS builds
if [ "$ACTION" = installhdrs ]; then exit 0; fi
if [ "${PLATFORM_NAME/iphone/}" != "${PLATFORM_NAME}" ]; then exit 0; fi

# open source plist
OSV="$DSTROOT"/usr/local/OpenSourceVersions
OSL="$DSTROOT"/usr/local/OpenSourceLicenses
mkdir -p -m 0755 "$OSV" "$OSL"
install -o "$INSTALL_OWNER" -g "$INSTALL_GROUP" -m 0444 removefile.plist "$OSV"
install -o "$INSTALL_OWNER" -g "$INSTALL_GROUP" -m 0444 LICENSE "$OSL"/removefile.txt

function InstallManPages() {
	for MANPAGE in "$@"; do
		SECTION=`basename "${MANPAGE/*./}"`
		MANDIR="$DSTROOT"/usr/share/man/man"$SECTION"
		install -d -o "$INSTALL_OWNER" -g "$INSTALL_GROUP" -m 0755 "$MANDIR"
		install -o "$INSTALL_OWNER" -g "$INSTALL_GROUP" -m 0444 "$MANPAGE" "$MANDIR"
	done
}

function LinkManPages() {
	MANPAGE=`basename "$1"`
	SECTION=`basename "${MANPAGE/*./}"`
	MANDIR="$DSTROOT"/usr/share/man/man"$SECTION"
	shift
	for LINK in "$@"; do
		ln -hf "$MANDIR/$MANPAGE" "$MANDIR/$LINK"
	done
}

InstallManPages removefile.3
LinkManPages removefile.3 \
	removefile_state_alloc.3 \
	removefile_state_free.3 \
	removefile_state_get.3 \
	removefile_state_set.3

InstallManPages checkint.3 
LinkManPages checkint.3 \
	check_{,u}int{32,64}_{add,sub,mul,div}.3
