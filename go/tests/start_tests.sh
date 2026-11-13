#!/bin/sh
# Builds and runs econf_test.go and uapi_test.go against the pure-Go
# implementation. No cgo or C library build is required.
set -e

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
cd "$script_dir"
exec go test -v ./...
