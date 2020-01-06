#!/bin/sh
# Copyright (c) 2018-2019 Petr Vorel <pvorel@suse.cz>
set -e

yum -y install \
	autoconf \
	automake \
	make \
	clang \
	gcc \
	findutils \
	libtirpc \
	pkg-config \
	redhat-lsb-core
