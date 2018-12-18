#!/bin/sh
# Copyright (c) 2018 Petr Vorel <pvorel@suse.cz>
set -e

./travis.$DISTRO.sh
../build.sh -o $TREE -t native -c $CC
