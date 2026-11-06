# Go bindings for libeconf

`econf/` is a Go module providing idiomatic bindings for
[libeconf](https://github.com/openSUSE/libeconf) via cgo. It wraps the
existing C library (see `include/libeconf.h`); it does not reimplement
libeconf's parsing/merging logic in Go.

## Prerequisites

Building this module requires the libeconf shared library and development
headers to be installed and discoverable via `pkg-config --libs libeconf`.
Either install the `libeconf` + `libeconf-devel` packages, or point
`PKG_CONFIG_PATH` at a local build, e.g.:

```sh
PKG_CONFIG_PATH=/path/to/libeconf/build go build ./...
```

If more than one libeconf is installed on the system (e.g. an older
distro-packaged version alongside a newer local build), make sure the linker
actually picks up the one you built against, for example:

```sh
CGO_LDFLAGS="-L/usr/local/lib64 -Wl,-rpath,/usr/local/lib64" go build ./...
```

## Usage

```go
package main

import (
	"fmt"
	"log"

	"github.com/openSUSE/libeconf/bindings/go/econf"
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

## API surface

The package mirrors the current (non-deprecated) public API from
`include/libeconf.h`:

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

Deliberately not wrapped: the deprecated `econf_readDirs`/
`econf_readDirsWithCallback` functions, and the non-thread-safe global
setters (`econf_requireOwner`, `econf_requireGroup`, `econf_requirePermissions`,
`econf_followSymlinks`, `econf_reset_security_settings`,
`econf_set_conf_dirs`) — use the callback-based read functions instead.

## Testing

```sh
cd econf
go test ./...
```

Tests are self-contained: they write small `.conf` fixtures into a temp
directory and exercise reading, writing, merging, getters/setters and the
callback-based read variant.
