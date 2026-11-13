# Go implementation of libeconf

`econf/` is a Go module providing a pure-Go reimplementation of
[libeconf](https://github.com/openSUSE/libeconf)'s parsing and merging
logic. It does not use cgo and does not link against the C library at all.

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

Man pages for the package are available in [`doc/man/`](doc/man/). Start
at `econf_go(3)` for an overview, e.g.:

```sh
man go/doc/man/econf_go.3
```

## API surface

The package covers the same (current, non-deprecated) functionality as the
public API in `include/libeconf.h`:

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

Deliberately not implemented: the deprecated
`econf_readDirs`/`econf_readDirsWithCallback` functions, the extended
`econf_getExtValue`/`econf_setExtValue` API, and the non-thread-safe global
setters (`econf_requireOwner`, `econf_requireGroup`, `econf_requirePermissions`,
`econf_followSymlinks`, `econf_reset_security_settings`,
`econf_set_conf_dirs`). `(*File).Close` exists only for API compatibility
with Go's usual `io.Closer` convention; it is a no-op since there is no C
memory to release.

## Examples

A runnable example lives in [`examples/`](examples/):

```sh
cd go/examples
./start.sh
```

## Testing

The test suite lives in [`tests/`](tests/):

```sh
cd go/tests
./start_tests.sh
```

The tests are ported from the upstream tst-uapi-* C tests using the fixtures
in tests/tst-*-data, keeping the behavior identical where Go allows.

