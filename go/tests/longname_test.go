package econf_test

import (
	"path/filepath"
	"testing"

	"github.com/openSUSE/libeconf/go/econf"
)

// TestLongKeyName replicates tst-long-name.c: a key name far longer than a
// typical I/O buffer size (8200 bytes here) is read correctly, not
// truncated.
func TestLongKeyName(t *testing.T) {
	path := filepath.Join("..", "..", "..", "tests", "tst-long-name", "test.conf")
	f, err := econf.ReadFile(path, "=", "#")
	if err != nil {
		t.Fatalf("ReadFile: %v", err)
	}
	defer f.Close()

	keys, err := f.Keys("")
	if err != nil {
		t.Fatalf("Keys: %v", err)
	}
	if len(keys) != 1 {
		t.Fatalf("Keys() returned %d keys, want 1", len(keys))
	}

	key := keys[0]
	if len(key) != 8200 {
		t.Errorf("len(key) = %d, want 8200", len(key))
	}

	got, err := f.String("", key)
	if err != nil {
		t.Fatalf("String(key): %v", err)
	}
	if got != "b" {
		t.Errorf("String(key) = %q, want \"b\"", got)
	}
}
