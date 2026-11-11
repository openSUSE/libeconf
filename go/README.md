# Go implementations of libeconf

This directory contains two independent Go implementations of
[libeconf](https://github.com/openSUSE/libeconf), both sharing the same
package name and import path - `github.com/openSUSE/libeconf/go/econf` -
and the same exported API. They are meant to be interchangeable: a
consuming project depends on that one import path without caring which
implementation backs it (e.g. two conflicting RPM packages could each
provide it, letting the system choose).

- [`binding/`](binding/README.md) - cgo bindings wrapping the existing C
  library (`include/libeconf.h`). Requires the libeconf shared library and
  headers at build time.
- [`native/`](native/README.md) - a native, pure-Go reimplementation of
  libeconf's parsing and merging logic. No cgo, no C library dependency.
- [`tests/`](tests/) - shared black-box test files (`econf_test.go`,
  `uapi_test.go`), run against each implementation from its own
  `tests/binding` and `tests/native` module (via `start_tests.sh` in each),
  to guarantee both satisfy the same contract.
- [`examples/`](examples/) - a shared runnable example (`example.go`), run
  against each implementation from its own `examples/binding` and
  `examples/native` module (via `start.sh` in each).
- [`doc/man/`](doc/man/) - man pages that are pure API reference and apply
  equally to both implementations, symlinked into `binding/doc/man` and
  `native/doc/man`. The package overview (`econf_go.3`) and
  `econf_go_File_Close.3` differ per implementation (cgo/C-memory details
  vs. a no-op `Close`), so each keeps its own copy of those two.

See each subdirectory's README for usage, prerequisites and testing
instructions.
