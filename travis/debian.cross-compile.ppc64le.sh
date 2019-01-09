#!/bin/sh
# Copyright (c) 2018 Petr Vorel <pvorel@suse.cz>
set -e

dpkg --add-architecture ppc64el
apt update

apt install -y --no-install-recommends \
	gcc-powerpc64le-linux-gnu \
	libc6-dev-ppc64el-cross \
	libmnl0:ppc64el \
	libmnl-dev:ppc64el \
	pkg-config:ppc64el
