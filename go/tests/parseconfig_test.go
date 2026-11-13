package econf_test

import (
	"path/filepath"
	"testing"

	"github.com/openSUSE/libeconf/go/econf"
)

// TestParseConfigEmptyGroup replicates tst-parseconfig1.c: a file
// containing only an empty group header parses without error.
func TestParseConfigEmptyGroup(t *testing.T) {
	path := filepath.Join("..", "..", "..", "tests", "tst-parseconfig-data", "empty-group.conf")
	f, err := econf.ReadFile(path, "=", "#")
	if err != nil {
		t.Fatalf("ReadFile: %v", err)
	}
	defer f.Close()
}

// TestParseConfigCommentCharInCommentLine replicates tst-parseconfig2.c: an
// /etc/os-release-style file with full-line comments parses, and a value
// is read back without a trailing newline leaking in.
func TestParseConfigCommentCharInCommentLine(t *testing.T) {
	path := filepath.Join("..", "..", "..", "tests", "tst-parseconfig-data", "osrelease.conf")
	f, err := econf.ReadFile(path, "=", "#")
	if err != nil {
		t.Fatalf("ReadFile: %v", err)
	}
	defer f.Close()

	got, err := f.String("", "ANSI_COLOR")
	if err != nil {
		t.Fatalf("String(ANSI_COLOR): %v", err)
	}
	if len(got) > 0 && got[len(got)-1] == '\n' {
		t.Errorf("String(ANSI_COLOR) = %q, has a trailing newline", got)
	}
	if got != "0;32" {
		t.Errorf("String(ANSI_COLOR) = %q, want \"0;32\"", got)
	}
}
