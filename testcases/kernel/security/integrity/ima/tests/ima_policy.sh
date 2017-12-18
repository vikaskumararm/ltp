#!/bin/sh
# Copyright (c) 2009 IBM Corporation
# Copyright (c) 2017 Petr Vorel <pvorel@suse.cz>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it would be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#
# Author: Mimi Zohar, zohar@ibm.vnet.ibm.com
# Author: Petr Vorel <pvorel@suse.cz>
#
# Test replacing the default integrity measurement policy.

TST_TESTFUNC="test"
TST_CNT=3
TST_CNT=1 # FIXME: debug
. ima_setup.sh

init()
{
	IMA_POLICY="$IMA_DIR/policy"
	[ -f $IMA_POLICY ] || tst_res TINFO "default policy already replaced"

	VALID_POLICY="$LTPROOT/testcases/data/ima_policy/measure.policy"
	echo "VALID_POLICY: '$VALID_POLICY'" # FIXME: debug
	[ -f $VALID_POLICY ] || tst_res TINFO "missing $VALID_POLICY"

	INVALID_POLICY="$LTPROOT/testcases/data/ima_policy/measure.policy-invalid"
	echo "INVALID_POLICY: '$INVALID_POLICY'" # FIXME: debug
	[ -f $INVALID_POLICY ] || tst_res TINFO "missing $INVALID_POLICY"
}

load_policy()
{
	exec 2>/dev/null 4>$IMA_POLICY
	if [ $? -ne 0 ]; then
		exit 1
	fi

	cat $1 |
	while read line ; do
	{
		if [ "${line#\#}" = "${line}" ] ; then
			echo $line >&4 2> /dev/null
			if [ $? -ne 0 ]; then
				exec 4>&-
				return 1
			fi
		fi
	}
	done
}


# Description   - Verify invalid policy doesn't replace default policy.
test1()
{
	load_policy $INVALID_POLICY & p1=$!
	wait "$p1"
	if [ $? -ne 0 ]; then
		tst_res TPASS "didn't load invalid policy"
	else
		tst_res TFAIL "loaded invalid policy"
	fi
}

# Description	- Verify policy file is opened sequentially, not concurrently
#		  and install new policy
test2()
{
	load_policy $VALID_POLICY & p1=$!  # forked process 1
	load_policy $VALID_POLICY & p2=$!  # forked process 2
	wait "$p1"; RC1=$?
	wait "$p2"; RC2=$?
	if [ $RC1 -eq 0 ] && [ $RC2 -eq 0 ]; then
		tst_res TFAIL "measurement policy opened concurrently"
	elif [ $RC1 -eq 0 ] || [ $RC2 -eq 0 ]; then
		tst_res TPASS "replaced default measurement policy"
	else
		tst_res TFAIL "problems opening measurement policy"
	fi
}

# Description 	- Verify can't load another measurement policy.
test3()
{
	load_policy $INVALID_POLICY & p1=$!
	wait "$p1"
	if [ $? -ne 0 ]; then
		tst_res TPASS "didn't replace valid policy"
	else
		tst_res TFAIL "replaced valid policy"
	fi
}

. ima_setup.sh

setup
init
tst_run
