#!/bin/sh
# Copyright (c) 2019 Petr Vorel <petr.vorel@gmail.com>
set -e

apk update

apk add \
	acl-dev \
	autoconf \
	automake \
	clang \
	gcc \
	keyutils-dev \
	libaio-dev \
	libacl \
	libcap-dev \
	libselinux-dev \
	libsepol-dev \
	libtirpc-dev \
	linux-headers \
	make \
	musl-dev \
	numactl-dev \
	openssl-dev \
	pkgconfig

echo "=== /etc/os-release ==="
cat /etc/os-release
