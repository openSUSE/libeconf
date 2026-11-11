// Package econf provides Go bindings for libeconf, a library for reading
// and merging key=value configuration file snippets from different
// directories (vendor, runtime, admin overrides).
//
// This package is a native, pure-Go reimplementation of libeconf's
// parsing/merging logic; it does not use cgo or link against the C
// library. It exposes the same package name, exported API and import path
// as the cgo-based binding in go/binding/econf, so the two are drop-in
// alternatives of each other: a consuming project can depend on
// "github.com/openSUSE/libeconf/go/econf" without caring which
// implementation is actually installed.
//
// See the upstream C API documented in include/libeconf.h for the exact
// semantics each function mirrors.
package econf
