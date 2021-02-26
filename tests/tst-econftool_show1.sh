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

econftool_exe="$PWD/../util/econftool"
if ! [[ -f "$econftool_exe" ]]; then
    econftool_exe="$PWD/econftool"
    if ! [[ -f "$econftool_exe" ]]; then
        echo "Couldn't find the econftool executable"
        exit 1
    fi
fi
econftool_root="$PWD/../../tests/tst-econftool-data"
if ! [[ -d "$econftool_root" ]]; then
    econftool_root="$PWD/../tests/tst-econftool-data"
    if ! [[ -d "$econftool_root" ]]; then
        echo "Couldn't set ECONFTOOL_ROOT"
        exit 1
    fi
fi
export ECONFTOOL_ROOT=$econftool_root

got_error=false

for ((i=0; i<${teststringslength}; i++)); do
    error=$($econftool_exe show ${teststrings[$i]} 2>&1)
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
