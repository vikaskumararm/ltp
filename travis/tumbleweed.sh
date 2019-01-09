#!/bin/sh
# Copyright (c) 2018 Petr Vorel <pvorel@suse.cz>
set -e

zypper --non-interactive install --no-recommends \
	autoconf \
	automake \
	clang \
	gcc \
	gzip \
	make \
	pkg-config \
	kernel-default-devel \
	keyutils-devel \
	libacl-devel \
	libaio-devel \
	libcap-devel \
	libnl3-devel \
	libnl3-200 \
	libnuma-devel \
	libopenssl-devel \
	libselinux-devel \
	libtirpc-devel \
	linux-glibc-devel \
	lsb-release
