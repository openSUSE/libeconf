// Package econf provides Go bindings for libeconf, a C library for reading
// and merging key=value configuration file snippets from different
// directories (vendor, runtime, admin overrides).
//
// This package wraps the existing C library via cgo; it does not
// reimplement libeconf's parsing/merging logic in Go. Building it requires
// the libeconf shared library and development headers to be installed and
// discoverable via pkg-config, e.g.:
//
//	PKG_CONFIG_PATH=/path/to/libeconf/build go build ./...
//
// See the upstream C API documented in include/libeconf.h for the exact
// semantics each wrapped function mirrors.
package econf
