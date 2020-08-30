#!/bin/bash


declare -a teststrings=(""
                        "-h"
                        "-j"
                        "-f"
                        "randomstring.conf"
                        "-f randomstring.conf"
                        "randomstring randomstring.conf")
declare -a experr=("Usage: econftool"
                   "Usage: econftool"
                   "invalid option -- 'j'"
                   "Invalid number of Arguments"
                   "Invalid number of Arguments"
                   "Invalid number of Arguments"
                   "Unknown command!")


teststringslength=${#teststrings[@]}

econftool_exe="$PWD/../util/econftool"
if ! [[ -f "$econftool_exe" ]]; then
    econftool_exe="$PWD/econftool"
    if ! [[ -f "$econftool_exe" ]]; then
        echo "Couldn't find the econftool executable"
        exit 1
    fi
fi

got_error=false

for ((i=0; i<${teststringslength}; i++)); do
    error=$($econftool_exe ${teststrings[$i]} 2>&1)
    if [[ ! $error =~ ${experr[$i]} ]]; then
        echo error for: ${teststrings[$i]}
        echo expected to contain: ${experr[$i]}
        echo got: $error
        got_error=true
    fi
done
if $got_error; then
    exit 1
fi
