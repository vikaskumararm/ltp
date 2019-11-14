#!/bin/sh
# Copyright (c) 2018 Petr Vorel <pvorel@suse.cz>

zypper --version

zypper --non-interactive install --no-recommends \
	autoconf \
	automake \
	clang \
	gcc \
	gzip \
	make \
	kernel-default-devel \
	keyutils-devel \
	libacl-devel \
	libaio-devel \
	libcap-devel \
	libnuma-devel \
	libopenssl-devel \
	libselinux-devel \
	libtirpc-devel \
	linux-glibc-devel \
	lsb-release
ret=$?

echo "=== START /var/log/zypper.log ==="
cat /var/log/zypper.log
echo "=== END /var/log/zypper.log ==="
exit $ret
