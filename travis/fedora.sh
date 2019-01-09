#!/bin/sh
# Copyright (c) 2018-2019 Petr Vorel <pvorel@suse.cz>
set -e

echo "0: $0, $(basename $0)" # FIXME: debug

echo "yum repolist"
yum repolist

yum -y install \
	autoconf \
	automake \
	make \
	clang \
	gcc \
	findutils \
	libmnl \
	pkg-config \
	redhat-lsb-core
