package econf_test

import (
	"errors"
	"testing"

	"github.com/openSUSE/libeconf/go/econf"
)

// TestOptions replicates the portable parts of tst-options.c: an unknown
// option is rejected with ErrOptionNotFound, an empty options string
// defaults DEFAULT_QUOTES to false, and "DEFAULT_QUOTES=1" turns it on.
//
// Not ported: the JOIN_SAME_ENTRIES/PYTHON_STYLE/PARSING_DIRS flag checks.
// The C test reads them directly off the internal econf_file struct (via
// the library's private lib/keyfile.h), which has no public Go
// equivalent. Worse, none of those three options (nor CONFIG_DIRS or
// ROOT_PREFIX) are reachable from the public Go API at all: there is no
// exported method to parse a file into an already options-configured
// *File (readFile/readFileWithCallback are unexported), and ReadConfig/
// ReadDirsHistory always build their own fresh *File internally rather
// than consuming one from NewKeyFileWithOptions. Only DEFAULT_QUOTES is
// actually wired up, because SetString/String run directly on the same
// *File instance that NewKeyFileWithOptions returned.
func TestOptions(t *testing.T) {
	t.Run("default quotes off by default", func(t *testing.T) {
		f, err := econf.NewKeyFileWithOptions("")
		if err != nil {
			t.Fatalf("NewKeyFileWithOptions(\"\"): %v", err)
		}
		defer f.Close()
		if err := f.SetString("group", "key", "value"); err != nil {
			t.Fatalf("SetString: %v", err)
		}
		if f.HasQuotes("group", "key") {
			t.Error("HasQuotes() = true, want false (DEFAULT_QUOTES not set)")
		}
	})

	t.Run("DEFAULT_QUOTES=1", func(t *testing.T) {
		f, err := econf.NewKeyFileWithOptions("DEFAULT_QUOTES=1")
		if err != nil {
			t.Fatalf("NewKeyFileWithOptions(DEFAULT_QUOTES=1): %v", err)
		}
		defer f.Close()
		if err := f.SetString("group", "key", "value"); err != nil {
			t.Fatalf("SetString: %v", err)
		}
		if !f.HasQuotes("group", "key") {
			t.Error("HasQuotes() = false, want true (DEFAULT_QUOTES=1)")
		}
	})

	t.Run("unknown option", func(t *testing.T) {
		_, err := econf.NewKeyFileWithOptions("PARSING_DIRS=/test/usr/etc/:/test/run:test/etc;UNKNOWN_TAG=1")
		if !errors.Is(err, econf.ErrOptionNotFound) {
			t.Errorf("NewKeyFileWithOptions(unknown tag) error = %v, want ErrOptionNotFound", err)
		}
	})
}
