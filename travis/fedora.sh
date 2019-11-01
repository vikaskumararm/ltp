#!/bin/sh
# Copyright (c) 2018-2020 Petr Vorel <pvorel@suse.cz>
set -e

yum -y install \
	autoconf \
	automake \
	make \
	clang \
	gcc \
	findutils \
	libmnl \
	libmnl-devel \
	pkg-config \
	redhat-lsb-core
