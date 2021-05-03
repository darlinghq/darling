#!/bin/bash

SCRIPT_DIR="$(dirname "$(readlink -f "$0")")"

"$SCRIPT_DIR/../src/external/swift/post-checkout.sh"
