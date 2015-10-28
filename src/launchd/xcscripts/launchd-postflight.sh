#!/bin/sh

mkdir -p "$DSTROOT/private/var/db/launchd.db/com.apple.launchd"
chown root:wheel "$DSTROOT/private/var/db/launchd.db"
chown root:wheel "$DSTROOT/private/var/db/launchd.db/com.apple.launchd"

mkdir -p "$DSTROOT/private/var/log/com.apple.launchd"
chown root:wheel "$DSTROOT/private/var/log/com.apple.launchd"

# These directories need to be here to satisfy certain third-party dependencies.
mkdir -p "$DSTROOT/private/etc/mach_init.d"
mkdir -p "$DSTROOT/private/etc/mach_init_per_user.d"
mkdir -p "$DSTROOT/private/etc/mach_init_per_login_session.d"
chown root:wheel "$DSTROOT/private/etc/mach_init.d"
chown root:wheel "$DSTROOT/private/etc/mach_init_per_user.d"
chown root:wheel "$DSTROOT/private/etc/mach_init_per_login_session.d"
