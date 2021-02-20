#!/bin/sh

tmpfile=/tmp/test_reference_output$$.txt

cleanup()
{
    if [ -f "${tmpfile}" ]; then
	echo removing "${tmpfile}"
	rm -f "${tmpfile}"
    fi
    exit 1
}

trap cleanup 1 2 3 15

run_program()
{
    program="${1}"
    filter="${2}"
    if [ "${filter}" = "" ]; then
	${program}
    else
	${program} | ${filter}
    fi
}

usage()
{
    echo "usage: $0 ( create | test ) <program> <reference_file> [ <filter> ]"
    exit 1
}

command="${1}"
program="${2}"
ref_file="${3}"
filter="${4}"

if [ "${ref_file}" = "" ]; then
    usage "${0}"
fi

if [ "${program}" = "" ]; then
    usage "${0}"
fi

if [ ! -f "${program}" ]; then
    echo "program '${program}' doesn't exit"
    exit 1
fi
if [ ! -x "${program}" ]; then
    program="sh ${program}"
fi
# echo "The program is ${program}"

if [ ! "${filter}" = "" ]; then
    if [ ! -f "${filter}" ]; then
	echo "filter '${filter}' doesn't exit"
	exit 1
    fi
    if [ ! -x "${filter}" ]; then
	filter="sh ${filter}"
    fi
#    echo "The filter is ${filter}"
else
    echo no filter
fi

if [ "${command}" = "create" ]; then
    run_program "${program}" "${filter}" > "${ref_file}"
elif [ "${command}" = "test" ]; then
    if [ ! -f "${ref_file}" ]; then
	echo "reference file '${ref_file}' doesn't exist"
	exit 1
    fi
    run_program "${program}" "${filter}" > "${tmpfile}"
    cmp "${tmpfile}" "${ref_file}"
    if [ ! $? -eq 0 ]; then
	echo FAILURE
	diff "${tmpfile}" "${ref_file}"
	exit 1
    fi
    echo SUCCESS
else
    usage "${0}"
fi

exit 0
