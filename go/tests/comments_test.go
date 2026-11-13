package econf_test

import (
	"path/filepath"
	"testing"

	"github.com/openSUSE/libeconf/go/econf"
)

// TestCommentCharacterInsideQuotedString replicates tst-comments2.c: a "#"
// inside a quoted value is part of the value, not the start of a comment,
// while a "#" outside quotes still starts one (and everything after it,
// including further "#" characters, is ignored).
//
// The upstream test locates tst-comments2-data/env.conf via
// econf_readConfig with the ROOT_PREFIX option; that option isn't part of
// either Go binding's public surface (see the equivalent note in
// TestUAPIFixtures), so this reads the fixture file directly instead. That
// changes only how the file is found, not the comment-parsing behavior
// under test.
//
// Not ported: tst-comments.c, which checks comment text itself via
// econf_getExtValue/econf_ext_value. That extended API is deliberately not
// exposed by either Go package (see go/native/README.md).
func TestCommentCharacterInsideQuotedString(t *testing.T) {
	path := filepath.Join("..", "..", "..", "tests", "tst-comments2-data", "env.conf")
	f, err := econf.ReadFile(path, "=", "#")
	if err != nil {
		t.Fatalf("ReadFile: %v", err)
	}
	defer f.Close()

	tests := []struct{ key, want string }{
		{"KEY1", "a#b"},
		{"KEY2", "a#b"},
		{"KEY3", "a"},
		{"KEY4", "test with #"},
	}
	for _, tc := range tests {
		got, err := f.String("", tc.key)
		if err != nil {
			t.Errorf("String(%q): %v", tc.key, err)
			continue
		}
		if got != tc.want {
			t.Errorf("String(%q) = %q, want %q", tc.key, got, tc.want)
		}
	}
}
