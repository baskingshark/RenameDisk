#!/bin/sh
#
# Copyright (c) 2014, baskingshark
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
#  buildpkg.sh
#

KEXT_FILE="$BUILT_PRODUCTS_DIR/RenameDisk.kext"
SCRIPTS_DIR="$SOURCE_ROOT/package/scripts"
PKG_FILE="$BUILT_PRODUCTS_DIR/$TARGET_NAME"

build_package() {
    pkgbuild \
        --component "$KEXT_FILE" \
        --install-location "$DEFAULT_KEXT_INSTALL_PATH" \
        --scripts "$SCRIPTS_DIR" \
        "$PKG_FILE"
}

case ${1:-build} in
build)
    build_package
    ;;
clean)
    rm -f "$PKG_FILE"
    ;;
install)
    build_package
    cp "$PKG_FILE" "$INSTALL_DIR"
    ;;
*)
    echo >&2 "Unknown action: \'$1\'"
    exit 1
esac