#!/bin/sh
# Copyright (c) 2018 Petr Vorel <pvorel@suse.cz>
set -e

yum update -y
yum -y install \
	autoconf \
	automake \
	make \
	clang \
	gcc \
	glibc-devel \
	findutils \
	redhat-lsb-core
