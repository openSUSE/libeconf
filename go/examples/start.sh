#!/bin/sh
# Builds and runs example.go against the pure-Go implementation. No cgo or
# C library build is required.
set -e

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
cd "$script_dir"
exec go run .
