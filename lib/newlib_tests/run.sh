#!/bin/sh

# blacklist directory content which is not a test
FILTER_OUT_FILES="$(basename $0) *.c config[0-9]* Makefile test_kconfig"

grep_pattern=
for i in $(echo $FILTER_OUT_FILES); do
	grep_pattern="-e $i $grep_pattern"
done

for i in *; do
	if ! echo $i | grep -q $grep_pattern; then
		echo "===== $i ====="
		./$i
		echo
	fi
done
