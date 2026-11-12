package econf_test

import (
	"errors"
	"path/filepath"
	"testing"

	"github.com/openSUSE/libeconf/go/econf"
)

// TestWriteStringDataRoundTrip replicates tst-write-string-data.c: writing
// to a directory that doesn't exist fails with ErrNoFile; writing to a
// real directory and reading the result back preserves every value
// (empty, quoted-empty, quoted, bare, and space- or word-containing
// strings); and merging a freshly created file that sets one key to an
// empty string does not crash.
func TestWriteStringDataRoundTrip(t *testing.T) {
	path := filepath.Join("..", "..", "..", "tests", "tst-write-string-data", "usr", "etc", "tst-string-data.conf")
	f, err := econf.ReadFile(path, "=", "#")
	if err != nil {
		t.Fatalf("ReadFile: %v", err)
	}
	defer f.Close()

	if err := f.WriteFile("not_exists", "out.ini"); !errors.Is(err, econf.ErrNoFile) {
		t.Errorf("WriteFile(nonexistent dir) error = %v, want ErrNoFile", err)
	}

	dir := t.TempDir()
	if err := f.WriteFile(dir, "out.ini"); err != nil {
		t.Fatalf("WriteFile: %v", err)
	}
	roundTripped, err := econf.ReadFile(filepath.Join(dir, "out.ini"), "=", "#")
	if err != nil {
		t.Fatalf("ReadFile(roundtrip): %v", err)
	}
	defer roundTripped.Close()

	for _, key := range []string{"test", "test2", "test3", "test4", "test5", "test6"} {
		orig, origErr := f.String("test", key)
		got, gotErr := roundTripped.String("test", key)
		if (origErr == nil) != (gotErr == nil) {
			t.Errorf("String(%q): original err=%v, roundtrip err=%v", key, origErr, gotErr)
			continue
		}
		if orig != got {
			t.Errorf("String(%q) roundtrip = %q, want %q", key, got, orig)
		}
	}

	fresh, err := econf.NewIniFile()
	if err != nil {
		t.Fatalf("NewIniFile: %v", err)
	}
	defer fresh.Close()
	if err := fresh.SetString("test", "test", ""); err != nil {
		t.Fatalf("SetString: %v", err)
	}

	merged, err := econf.MergeFiles(fresh, f)
	if err != nil {
		t.Fatalf("MergeFiles: %v", err)
	}
	defer merged.Close()
}
