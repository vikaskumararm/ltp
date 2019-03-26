#!/bin/sh
# Copyright (c) 2018-2019 Petr Vorel <pvorel@suse.cz>
set -e

# workaround for missing oldstable-updates repository
# W: Failed to fetch http://deb.debian.org/debian/dists/oldstable-updates/main/binary-amd64/Packages
grep -v oldstable-updates /etc/apt/sources.list > /tmp/sources.list && mv /tmp/sources.list /etc/apt/sources.list

# FIXME: debug
echo "pev: in debian.sh" # FIXME: debug
echo "run: grep -rl oldstable-updates /etc/apt/sources.list.d/ | xargs rm -fv" # FIXME: debug
echo "find /etc/apt/sources.list.d/"
find /etc/apt/sources.list.d/
for i in /etc/apt/sources.list /etc/apt/sources.list.d/*; do if [ -f "$i" ]; then echo "== $i =="; cat $i; fi; done
# FIXME: debug

apt update

apt install -y --no-install-recommends \
    acl-dev \
    autoconf \
    automake \
    build-essential \
    debhelper \
    devscripts \
    clang \
    gcc \
    libacl1 \
    libacl1-dev \
    libaio-dev \
    libaio1 \
    libcap-dev \
    libcap2 \
    libc6 \
    libc6-dev \
    libkeyutils-dev \
    libkeyutils1 \
    libmm-dev \
    libnuma-dev \
    libnuma1 \
    libselinux1-dev \
    libsepol1-dev \
    libssl-dev \
    linux-libc-dev \
    lsb-release

apt install libtirpc1 libtirpc3 || true
