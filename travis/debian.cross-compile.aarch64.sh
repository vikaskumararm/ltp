#!/bin/sh
# Copyright (c) 2018 Petr Vorel <pvorel@suse.cz>
set -e

dpkg --add-architecture arm64
apt update

apt install -y --no-install-recommends \
	gcc-aarch64-linux-gnu \
	libc6-dev-arm64-cross \
	libmnl0:arm64 \
	libmnl-dev:arm64
