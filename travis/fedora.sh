#!/bin/sh
# Copyright (c) 2018-2020 Petr Vorel <pvorel@suse.cz>
set -e

EXTRA_PACKAGES="libtirpc-devel"
case "$RPC" in
libntirpc) EXTRA_PACKAGES="libntirpc-devel";;
none) EXTRA_PACKAGES=;;
esac

yum -y install \
	autoconf \
	automake \
	make \
	clang \
	gcc \
	findutils \
	pkg-config \
	redhat-lsb-core \
	$EXTRA_PACKAGES
