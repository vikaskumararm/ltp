#!/bin/sh
# Copyright (c) 2018 Petr Vorel <pvorel@suse.cz>
set -e

./travis.debian.stable.sh

apt install -y --no-install-recommends \
	gcc-aarch64-linux-gnu \
	libc6-dev-arm64-cross
