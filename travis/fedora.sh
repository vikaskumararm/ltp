#!/bin/sh
# Copyright (c) 2018-2020 Petr Vorel <pvorel@suse.cz>
set -e

echo "search libmnl"
yum search libmnl
echo "-------------"

yum -y install \
	autoconf \
	automake \
	make \
	clang \
	gcc \
	findutils \
	pkg-config \
	redhat-lsb-core

# CentOS 8 doesn't have libmnl-devel
yum -y install libmnl-devel || yum -y install libmnl
