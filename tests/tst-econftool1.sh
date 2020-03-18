#!/bin/bash


declare -a teststrings=(""
                        "-h"
                        "-j"
                        "-f"
                        "-f randomstring.conf")
declare -a experr=("Usage: econftool"
                   "Usage: econftool"
                   "invalid option -- 'j'"
                   "Missing filename!"
                   "Unknown command!")


teststringslength=${#teststrings[@]}

got_error=false

for ((i=0; i<${teststringslength}; i++)); do
    error=$($PWD/../util/econftool ${teststrings[$i]} 2>&1)
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
