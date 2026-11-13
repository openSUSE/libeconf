package econf_test

import (
	"errors"
	"path/filepath"
	"strings"
	"testing"

	"github.com/openSUSE/libeconf/go/econf"
)

// TestOptions replicates the portable parts of tst-options.c: an unknown
// option is rejected with ErrOptionNotFound, an empty options string
// defaults DEFAULT_QUOTES to false, and "DEFAULT_QUOTES=1" turns it on.
//
// Not ported: the JOIN_SAME_ENTRIES/PYTHON_STYLE/PARSING_DIRS flag checks
// themselves, since the C test reads them directly off the internal
// econf_file struct (via the library's private lib/keyfile.h), which has
// no public Go equivalent. Their actual effect on parsing is exercised
// behaviorally instead - see TestJoinSameEntriesViaReadConfig here, and
// TestGetConfDirsCustomConfigAndParsingDirs / TestManualParsingDirs in
// getconfdirs_test.go / mandir_test.go for PARSING_DIRS/CONFIG_DIRS -
// all three via (*File).ReadConfig, which (unlike the package-level
// ReadConfig) reuses the receiver's own options instead of a fresh
// *File. ROOT_PREFIX remains unreachable from the public API: nothing
// consumes File.rootPrefix.
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

// TestJoinSameEntriesViaReadConfig exercises the JOIN_SAME_ENTRIES option
// behaviorally (the input.conf fixture tst-string-append.c also uses, but
// without that test's econf_getExtValue-based checks; see the skip note on
// tst-string-append.c in string_test.go): without the option, a key
// defined twice under the same group keeps only its first value; with
// JOIN_SAME_ENTRIES=1, the second occurrence's value is appended (newline
// separated) to the first instead.
func TestJoinSameEntriesViaReadConfig(t *testing.T) {
	base := filepath.Join("..", "..", "tests", "tst-append-string")

	withoutJoin, err := econf.NewKeyFileWithOptions("")
	if err != nil {
		t.Fatalf("NewKeyFileWithOptions(\"\"): %v", err)
	}
	defer withoutJoin.Close()
	if err := withoutJoin.ReadConfig("", base, "input", "conf", "=", "#"); err != nil {
		t.Fatalf("ReadConfig (no JOIN_SAME_ENTRIES): %v", err)
	}
	plain, err := withoutJoin.String("main", "append_option")
	if err != nil {
		t.Fatalf("String(append_option): %v", err)
	}

	withJoin, err := econf.NewKeyFileWithOptions("JOIN_SAME_ENTRIES=1")
	if err != nil {
		t.Fatalf("NewKeyFileWithOptions(JOIN_SAME_ENTRIES=1): %v", err)
	}
	defer withJoin.Close()
	if err := withJoin.ReadConfig("", base, "input", "conf", "=", "#"); err != nil {
		t.Fatalf("ReadConfig (JOIN_SAME_ENTRIES=1): %v", err)
	}
	joined, err := withJoin.String("main", "append_option")
	if err != nil {
		t.Fatalf("String(append_option): %v", err)
	}

	if joined == plain {
		t.Fatalf("JOIN_SAME_ENTRIES=1 had no effect: joined value %q equals the unjoined value", joined)
	}
	if !strings.Contains(joined, "three") || !strings.Contains(joined, "four") {
		t.Errorf("joined value %q is missing the second entry's content", joined)
	}
}
