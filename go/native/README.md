# Go bindings for libeconf (native)

`econf/` is a Go module providing a native, pure-Go reimplementation of
[libeconf](https://github.com/openSUSE/libeconf)'s parsing and merging
logic. Unlike `go/binding`, it does not use cgo and does not link against
the C library at all.

It intentionally shares its package name, import path
(`github.com/openSUSE/libeconf/go/econf`) and exported API with
`go/binding`: the two are meant to be interchangeable at the packaging
level (e.g. two conflicting RPM packages), so a project can depend on this
import path without caring which implementation is actually installed.

## Prerequisites

None beyond a Go toolchain - no C library, headers or `pkg-config` lookup
required.

```sh
go build ./...
```

## Usage

```go
package main

import (
	"fmt"
	"log"

	"github.com/openSUSE/libeconf/go/econf"
)

func main() {
	kf, err := econf.ReadConfig("example", "/usr/lib", "example", "conf", "=", "#")
	if err != nil {
		log.Fatal(err)
	}
	defer kf.Close()

	value, err := kf.StringDef("main", "some_key", "default_value")
	if err != nil {
		log.Println("falling back to default:", err)
	}
	fmt.Println(value)
}
```

## Documentation

Man pages for the package are available in `go/native/doc/man/`. Most of
them are pure API reference shared (via a symlink) with `go/binding`'s
identical documentation, living in `go/doc/man/`; `econf_go(3)` (the
overview) and `econf_go_File_Close(3)` are native-specific, since they
describe this package's no-cgo/no-op-Close details. Start at `econf_go(3)`
for an overview, e.g.:

```sh
man go/native/doc/man/econf_go.3
```

## API surface

The package covers the same (current, non-deprecated) functionality as the
public API in `include/libeconf.h`, using identical Go names and signatures
to `go/binding` so the two stay interchangeable:

- Constructors: `NewKeyFile`, `NewKeyFileWithOptions`, `NewIniFile`
- Reading: `ReadFile`, `ReadFileWithCallback`, `ReadConfig`,
  `ReadConfigWithCallback`, `ReadDirsHistory`, `ReadDirsHistoryWithCallback`,
  `MergeFiles`
- Writing: `(*File).WriteFile`
- Introspection: `(*File).Path`, `(*File).Groups`, `(*File).Keys`,
  `(*File).CommentTag`/`SetCommentTag`, `(*File).DelimiterTag`/`SetDelimiterTag`,
  `(*File).HasQuotes`
- Getters/setters for each value type (`Int`, `Int64`, `UInt`, `UInt64`,
  `Float`, `Double`, `String`, `Bool`), each getter with a `*Def` variant
  taking a default value, plus `SetQuotes`

Errors are returned as `*econf.Error`; compare against the sentinel values
(`econf.ErrNoKey`, `econf.ErrNoGroup`, ...) with `errors.Is`.

Deliberately not implemented, matching `go/binding`: the deprecated
`econf_readDirs`/`econf_readDirsWithCallback` functions, the extended
`econf_getExtValue`/`econf_setExtValue` API, and the non-thread-safe global
setters (`econf_requireOwner`, `econf_requireGroup`, `econf_requirePermissions`,
`econf_followSymlinks`, `econf_reset_security_settings`,
`econf_set_conf_dirs`). `(*File).Close` exists only for API compatibility;
it is a no-op since there is no C memory to release.

## Examples

A runnable example (`example.go`) lives in `go/examples` and is shared
(via a symlink) with `go/binding`'s equivalent example run:

```sh
cd go/examples/native
./start.sh
```

## Testing

This package has no tests of its own; both test files live in `go/tests`
and are run against each implementation (via a symlink) from its own
`go/tests/binding` and `go/tests/native` module, guaranteeing both satisfy
the same contract:

```sh
cd go/tests/native
./start_tests.sh
```

- `econf_test.go`: reading, writing, merging, getters/setters, the
  callback-based read variant.
- `uapi_test.go`: the directory-layering/merge engine (`ReadDirsHistory` +
  `MergeFiles`), checked against the expected results documented in several
  of the upstream `tst-uapi-*` C tests.

This package's behavior has additionally been differentially tested against
the cgo binding (backed by the real C library) across every fixture in
`tests/tst-*-data`.
