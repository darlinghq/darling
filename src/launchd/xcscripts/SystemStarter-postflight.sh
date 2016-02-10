#!/bin/sh

install -o "$INSTALL_OWNER" -g "$INSTALL_GROUP" -m 0755 -d "$DSTROOT"/System/Library/StartupItems
install -o "$INSTALL_OWNER" -g "$INSTALL_GROUP" -m 0755 -d "$DSTROOT"/Library/StartupItems
