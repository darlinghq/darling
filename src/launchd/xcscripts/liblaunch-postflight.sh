#!/bin/sh

install -o "$INSTALL_OWNER" -g "$INSTALL_GROUP" -m 0755 -d "$DSTROOT"/usr/include/servers
mv "$DSTROOT"/usr/include/bootstrap.h "$DSTROOT"/usr/include/servers/
ln -sf bootstrap.h "$DSTROOT"/usr/include/servers/bootstrap_defs.h
