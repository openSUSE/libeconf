#!/bin/bash


declare -a teststrings=("example.conf"
                      "example.conf -f"
                      "nonexistant.conf"
                      "randomstring")
declare -a experr=("variable = 301"
                   "variable = 301"
                   "Configuration file not found"
                   "Currently only works with a dot in the filename!")


teststringslength=${#teststrings[@]}

libeconfdir=$PWD
if [[ $PWD =~ "_build" ]]; then
	cd ../../..
	export ECONFTOOL_ROOT=$PWD/tests/tst-econftool-data
else
	export ECONFTOOL_ROOT=$PWD/tst-econftool-data
fi

got_error=false

for ((i=0; i<${teststringslength}; i++)); do
    error=$($libeconfdir/../util/econftool show ${teststrings[$i]} 2>&1)
    if [[ ! $error =~ ${experr[$i]} ]]; then
        echo error for ${teststrings[$i]}
        echo expected to contain: ${experr[$i]}
        echo got: $error
        got_error=true
    fi
done
if $got_error; then
    exit 1
fi
