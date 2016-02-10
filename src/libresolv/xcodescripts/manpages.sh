#!/bin/bash
set -e -x

# don't install man pages for installhdrs or iOS builds
if [ "$ACTION" = installhdrs ]; then exit 0; fi
if [ "${PLATFORM_NAME/iphone/}" != "${PLATFORM_NAME}" ]; then exit 0; fi

MANDIR="$DSTROOT"/usr/share/man

install -d -m 0755 -o "$INSTALL_OWNER" -g "$INSTALL_GROUP" \
	"$MANDIR"/man3 \
	"$MANDIR"/man5

install -c -m 0644 -o "$INSTALL_OWNER" -g "$INSTALL_GROUP" \
	"$SRCROOT"/resolver.3 \
	"$MANDIR"/man3

install -c -m 0644 -o "$INSTALL_OWNER" -g "$INSTALL_GROUP" \
	"$SRCROOT"/resolver.5 \
	"$MANDIR"/man5

set +x
echo "Install man pages..."

for M in dn_comp.3 \
	dn_expand.3 \
	dn_skipname.3 \
	ns_get16.3 \
	ns_get32.3 \
	ns_put16.3 \
	ns_put32.3 \
	res_init.3 \
	res_mkquery.3 \
	res_query.3 \
	res_search.3 \
	res_send.3 ; do
	echo "... $M"
	echo ".so man3/resolver.3" > "$MANDIR"/man3/"$M"
	chown "$INSTALL_OWNER:$INSTALL_GROUP" "$MANDIR"/man3/"$M"
	chmod 0644 "$MANDIR"/man3/"$M"
done
