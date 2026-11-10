#!/bin/sh
# Builds and runs example.go against the locally built libeconf
# (bindings/go/examples/../../../build), instead of the version installed
# on the system.
set -e

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
repo_root=$(CDPATH= cd -- "$script_dir/../../.." && pwd)

export CGO_CFLAGS="-I$repo_root/include"
export CGO_LDFLAGS="-L$repo_root/build/lib -Wl,-rpath,$repo_root/build/lib"
export LD_LIBRARY_PATH="$repo_root/build/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

cd "$script_dir"
exec go run .
