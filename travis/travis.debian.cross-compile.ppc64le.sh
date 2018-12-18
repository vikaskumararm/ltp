#!/bin/sh
# Copyright (c) 2018 Petr Vorel <pvorel@suse.cz>
set -e

./travis.debian.stable.sh

apt install -y --no-install-recommends \
	gcc-powerpc64le-linux-gnu \
	libc6-dev-ppc64el-cross
