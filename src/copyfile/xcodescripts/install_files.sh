#!/bin/sh
set -e -x

# don't install man pages for installhdrs or iOS builds
if [ "$ACTION" = installhdrs ]; then exit 0; fi
if [ "${PLATFORM_NAME/iphone/}" != "${PLATFORM_NAME}" ]; then exit 0; fi

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

InstallManPages copyfile.3
LinkManPages copyfile.3 \
	fcopyfile.3 \
	copyfile_state_alloc.3 \
	copyfile_state_free.3 \
	copyfile_state_get.3 \
	copyfile_state_set.3

InstallManPages xattr_name_with_flags.3
LinkManPages    xattr_name_with_flags.3 \
	xattr_name_without_flags.3 \
	xattr_flags_from_name.3 \
	xattr_intent_with_flags.3

