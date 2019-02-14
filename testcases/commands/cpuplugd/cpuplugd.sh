# Copyright (C) 2018 IBM Corp.
#
# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.  This file is offered as-is,
# without any warranty.


#!/usr/bin/env bash
###############################################################################
# The following changes were made in order to fix the cpuplugd daemon status
# verification and also to perform it before trying to stop the daemon:
#
# - serviceruns and assert_cpuplugd_not_running functions eliminated;
# - stop_cpuplugd function rebuilt;
# - assert_cpuplugd_running, assert_cpuplugd_not_running and serviceruns
# functions merged into new rebuilt assert_cpuplugd_running function;
# - changed all assert_cpuplugd_running function calls to use 2 arguments;
# - inserted assert_cpuplugd_running function calls before trying to stop
# cpuplugd daemon.
#
# Changes
# -------
# 07.03.2018  Corrections for SLES15 (no PID file when
#                     running in interactive mode)
#
###############################################################################
TST_CNT=11
TST_TESTFUNC=cpuplugd_test
. tst_test.sh

number_of_cpus=$(lscpu | grep -wF "CPU(s):" | awk 'FNR == 1 {print $2}')
(( SLEEP_X = number_of_cpus * 2 ))

# inherit all variables to subprocesses
set -a

readonly CMM_MOD="$(grep "CONFIG_CMM=" /boot/config-$(uname -r) | cut -d= -f2 | tr '[:upper:]' '[:lower:]')"

for f in lib/*.sh; do source $f; done
source ./cpuplugd_1.sh || exit 1

init_tests

cpuplugd_run()
{

        $1
        if [ $? -eq $2 ]; then
                tst_res TPASS "'$1' returned '$2'. Test passed"
        else
                tst_res TFAIL "'$1' did not return '$2'. Test failed"
        fi
}

cpuplugd_test1()
{
    echo "Cpuplugd version"
    cpuplugd=$(cpuplugd -v | head -n 1 | cut -d":" -f2 | cut -d" " -f10)
    cpuplugd_run "cpuplugd -v" 0
    echo "Cpuplugd version $cpuplugd"
}
cpuplugd_test2()
{
    echo "Cpuplugd Help information checking"
    echo "Checking for Help information with -h option"
    cpuplugd_run "cpuplugd -h" 0
    echo "Checking for Help information with --help option"
    cpuplugd_run "cpuplugd --help" 0
}
#
# Run several tests with incomplete/broken configuration.
# Correct behavior for the daemon is not to start.
#
cpuplugd_test3()
{
    echo "Invalid options checking"
    echo "Checking for Invalid option -H"
    cpuplugd_run "cpuplugd -H" 1
    echo "Checking for Invalid option -1234"
    cpuplugd_run "cpuplugd -1234" 1
}

cpuplugd_test4()
{
    echo "Run daemon without UPDATE entry in config"
    prepare_incomplete_cpu_test_config UPDATE
    cpu_test_with_error_in_cpuplgdtest_conf 1 1
}

cpuplugd_test5()
{
    echo "Run Daemon without CPU_MIN entry"
    prepare_incomplete_cpu_test_config CPU_MIN
    if isVM; then
        if [[ "${CMM_MOD}" = "m" || -z "${CMM_MOD}" ]]; then
            cpu_test_with_error_in_cpuplgdtest_conf 1 1
        elif [[ "${CMM_MOD}" = "y" ]]; then
            cpu_test_with_error_in_cpuplgdtest_conf 0 1
        fi
    else
        cpu_test_with_error_in_cpuplgdtest_conf 1 1
    fi
}

cpuplugd_test6()
{
    echo "Run Daemon without CPU_MAX entry"
    prepare_incomplete_cpu_test_config CPU_MAX
    cpu_test_with_error_in_cpuplgdtest_conf 1 1
}
cpuplugd_test7()
{
    echo "Run Daemon without HOTPLUG entry"
    prepare_incomplete_cpu_test_config HOTPLUG
    if isVM; then
        if [[ "${CMM_MOD}" = "m" || -z "${CMM_MOD}" ]]; then
            cpu_test_with_error_in_cpuplgdtest_conf 1 1
        elif [[ "${CMM_MOD}" = "y" ]]; then
            cpu_test_with_error_in_cpuplgdtest_conf 0 1
        fi
    else
        cpu_test_with_error_in_cpuplgdtest_conf 1 1
    fi
}
cpuplugd_test8()
{
    echo "Run Daemon without HOTUNPLUG entry"
    prepare_incomplete_cpu_test_config HOTUNPLUG
    if isVM; then
        if [[ "${CMM_MOD}" = "m" || -z "${CMM_MOD}" ]]; then
            cpu_test_with_error_in_cpuplgdtest_conf 1 1
        elif [[ "${CMM_MOD}" = "y" ]]; then
            cpu_test_with_error_in_cpuplgdtest_conf 0 1
        fi
    else
        cpu_test_with_error_in_cpuplgdtest_conf 1 1
    fi
}

#
# Good path test for CPU plugging
#
# Run daemon with only cpu configuration in configuration file
# CPU Hotplug  with  3 Active cpus and CPU_MIN=1 and CPU_MAX=3
# Verify that number of CPUs is reduced after daemon is started
# and restored after daemon is stopped.
#

cpuplugd_test9()
{
	echo  "Run Daemon with only cpu configuration"

        assert_cpuplugd_running 1 1

        cpusbefore=$(grep "processors" /proc/cpuinfo | cut -d":" -f2)
        echo "running daemon with only cpu config" > cpuf
        echo "start cpuplugd -V -c cpu.conf -f "
        cpuplugd  -V -c cpu.conf -f >> cpuf &
        sleep 2

        assert_cpuplugd_running 0 1
        RC=$?
        echo "Let Daemon run for few seconds, so that it does the hotplugging"
        sleep ${SLEEP_X}

        cpusrunning=$(grep "processors" /proc/cpuinfo | cut -d":" -f2)

        if [[ ${RC} -eq 0 ]]; then
            stop_cpuplugd
        fi

        sleep 2
        cpusafter=$(grep "processors" /proc/cpuinfo | cut -d":" -f2)
        i=0

        if (( "$cpusbefore" != "$cpusrunning" )) && (( "$cpusbefore" == "$cpusafter" )); then
            i=1
        fi

        assert_warn $i 1  "Verify that number of CPUs before, while and after the daemon runs changes: cpusbefore=$cpusbefore, cpusduring=$cpusrunning, cpusafterter=$cpusafter"
}

cpuplugd_test10()
{
#
# Memory Hotplug tests (z/VM only)
#
   if isVM; then
    #
    # Run several tests with incomplete/broken configuration.
    # Correct behavior for the daemon is not to start.
    #

        echo "Run Daemon without loading CMM module"
            if [[ "${CMM_MOD}" != "m" ]]; then
                assert_warn 0 0 "This test is not possible to be performed since CMM was builtin within the kernel and cannot be unloaded."
            else
                assert_cpuplugd_running 1 0
                RC=$?

                if [[ ${RC} -eq 0 ]]; then
                   stop_cpuplugd
                fi

                rmmod cmm
                lsmod | grep -q cmm
                assert_warn $? 1 "Verify that CMM module is not loaded"

                cpuplugd -V -c cmm.conf -f >> cmm.log &
                echo "Daemon should fail to run as CMM module is not loaded"
                sleep 2

                assert_cpuplugd_running 1 1
                RC=$?

                rm -rf cmm.log

                if [[ ${RC} -eq 0 ]]; then
                    stop_cpuplugd
                fi
            fi


        echo "Run Daemon without CMM_MIN entry"
            prepare_incomplete_cmm_test_config CMM_MIN
            cmm_test_with_error_in_cpuplgdtest_conf 1 1


        echo "Run Daemon without CMM_MAX entry"
            prepare_incomplete_cmm_test_config CMM_MAX
            cmm_test_with_error_in_cpuplgdtest_conf 1 1


        echo "Run Daemon without CMM_INC entry"
            prepare_incomplete_cmm_test_config CMM_INC
            cmm_test_with_error_in_cpuplgdtest_conf 1 1


        echo "Run Daemon without MEMPLUG entry"
            prepare_incomplete_cmm_test_config MEMPLUG
            cmm_test_with_error_in_cpuplgdtest_conf 1 1


        echo "Run Daemon without MEMUNPLUG entry"
            prepare_incomplete_cmm_test_config MEMUNPLUG
            cmm_test_with_error_in_cpuplgdtest_conf 1 1


#
# Good path test for CMM plugging
#
        echo "Run Daemon with only CMM configuration"
            if [[ "${CMM_MOD}" = "m" ]]; then
                modprobe cmm
            fi

            echo 1000 > /proc/sys/vm/cmm_pages
            sleep 1

            cmm_pages_before=$(cat /proc/sys/vm/cmm_pages)
            cpusbefore=$(grep "processors" /proc/cpuinfo | cut -d":" -f2)
            cpuplugd -V -c cmm.conf -f  >> cmm.log &
            sleep 2

            assert_cpuplugd_running 0 1
            RC=$?
            sleep 4

            cmm_pages_running=$(cat /proc/sys/vm/cmm_pages)
            cpusrunning=$(grep "processors" /proc/cpuinfo | cut -d":" -f2)

            if [[ ${RC} -eq 0 ]]; then
                stop_cpuplugd
            fi

            sleep 2

            cmm_pages_after=$(cat /proc/sys/vm/cmm_pages)
            cpusafter=$(grep "processors" /proc/cpuinfo | cut -d":" -f2)
            i=0

            if (( "$cmm_pages_before" != "$cmm_pages_running" )) && (( "$cmm_pages_before" == "$cmm_pages_after" )); then
                if (( "$cpusbefore" == "$cpusrunning" )) && (( "$cpusbefore" == "$cpusafter" )); then
                    i=1
                fi
            fi

            assert_warn $i 1  "Verify that the number of cmm_pages before, while and after the daemon runs changes:                 cmm_pages_before=$cmm_pages_before, cmm_pages_running=$cmm_pages_running, cmm_pages_after=$cmm_pages_after. CPU hotplugging should not happen and cpu configuration should remain constant: cpus_before=$cpusbefore, cpus_during=$cpusrunning, cpus_after=$cpusafter"

    else
        # if it is LPAR, Memory Hotplug cannot be done
        assert_exec 1 "modprobe vmcp"
        echo "This is LPAR. Skip CMM Tests"
    fi
}

cpuplugd_test11()
{
    echo "Doing the cleanup tasks ..."
    cpuplugd_run "service cpuplugd stop" 0

    if [[ "${CMM_MOD}" = "m" ]]; then
        cpuplugd_run "rmmod cmm" 0
    fi

    cpuplugd_run "rm -rf cpuf cmm log cmm.log cpuplugdtest.conf te.tes out.txt.tmp" 0

}

tst_run
