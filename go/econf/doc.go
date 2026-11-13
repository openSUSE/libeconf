// Package econf provides Go bindings for libeconf, a library for reading
// and merging key=value configuration file snippets from different
// directories (vendor, runtime, admin overrides).
//
// This is a pure-Go reimplementation of libeconf's parsing/merging logic;
// it does not use cgo or link against the C library.
//
// See the upstream C API documented in include/libeconf.h for the exact
// semantics each function mirrors.
package econf
