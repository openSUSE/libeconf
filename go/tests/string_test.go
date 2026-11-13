package econf_test

import (
	"path/filepath"
	"testing"

	"github.com/openSUSE/libeconf/go/econf"
)

// TestStringFormats replicates tst-string.c: several string value formats
// (empty, embedded spaces, quoted leading/trailing spaces, bare and
// quoted multi-line continuations, and escaped/quoted quote characters)
// all read back exactly as written.
func TestStringFormats(t *testing.T) {
	path := filepath.Join("..", "..", "tests", "tst-arguments-string", "etc", "arguments.conf")
	f, err := econf.ReadFile(path, "=", "#")
	if err != nil {
		t.Fatalf("ReadFile: %v", err)
	}
	defer f.Close()

	tests := []struct{ key, want string }{
		{"string_empty", ""},
		{"string_with_spaces", "string with spaces"},
		{"string_escaped_with_leading_and_trailing_spaces", " string with spaces "},
		{"string_with_newlines", "line one\n    line two"},
		{"string_list_multiple_lines", "\n    line one\n    line two"},
		{"string_escaped_with_newlines", "\"line one\n    line two\""},
		{"string_with_quotes", `\"`},
		{"string_with_quotes_v2", `\"`},
	}
	for _, tc := range tests {
		got, err := f.String("main", tc.key)
		if err != nil {
			t.Errorf("String(%q): %v", tc.key, err)
			continue
		}
		if got != tc.want {
			t.Errorf("String(%q) = %q, want %q", tc.key, got, tc.want)
		}
	}
}
