#!/bin/sh
# Copyright (c) 2018-2020 Petr Vorel <pvorel@suse.cz>
set -e

if [ -z "$ARCH" ]; then
	echo "missing \$ARCH!" >&2
	exit 1
fi

case "$ARCH" in
arm64) gcc_arch="aarch64";;
ppc64el) gcc_arch="powerpc64le";;
*) echo "unsupported arch: '$1'!" >&2; exit 1;;
esac

dpkg --add-architecture $ARCH
apt update

EXTRA_PACKAGES="libtirpc-dev:$ARCH"
case "$RPC" in
libntirpc) EXTRA_PACKAGES="libntirpc-dev:$ARCH";;
none) EXTRA_PACKAGES=;;
esac

apt install -y --no-install-recommends \
	gcc-${gcc_arch}-linux-gnu \
	libc6-dev-${ARCH}-cross \
	$EXTRA_PACKAGES
