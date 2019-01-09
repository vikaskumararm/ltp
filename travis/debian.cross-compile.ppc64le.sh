#!/bin/sh
# Copyright (c) 2018 Petr Vorel <pvorel@suse.cz>
set -e

dpkg --add-architecture ppc64el
apt update

apt install -y --no-install-recommends \
	gcc-powerpc64le-linux-gnu \
	libc6-dev-ppc64el-cross \
	pkg-config:ppc64el \
	libnl-3-dev:ppc64el \
	libnl-cli-3-dev:ppc64el \
	libnl-cli-3-200:ppc64el \
