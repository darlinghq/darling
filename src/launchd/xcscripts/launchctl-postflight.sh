#!/bin/sh

install -o "$INSTALL_OWNER" -g "$INSTALL_GROUP" -m 0755 -d "$DSTROOT"/System/Library/LaunchAgents
install -o "$INSTALL_OWNER" -g "$INSTALL_GROUP" -m 0755 -d "$DSTROOT"/System/Library/LaunchDaemons
install -o "$INSTALL_OWNER" -g "$INSTALL_GROUP" -m 0755 -d "$DSTROOT"/Library/LaunchAgents
install -o "$INSTALL_OWNER" -g "$INSTALL_GROUP" -m 0755 -d "$DSTROOT"/Library/LaunchDaemons
