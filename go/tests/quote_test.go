package econf_test

import (
	"errors"
	"os"
	"path/filepath"
	"strings"
	"testing"

	"github.com/openSUSE/libeconf/go/econf"
)

// TestQuotes replicates tst-quote1.c: reading a file full of quoted,
// unquoted and unbalanced-quote values and checking both the unquoted
// value and the HasQuotes flag for each, then exercising SetQuotes.
//
// Not ported: econf_hasQuotes(NULL, ...) and econf_setQuotes(NULL, ...)
// are C-specific NULL-key_file robustness checks (expected to report
// "false"/ECONF_FILE_LIST_IS_NULL without crashing); calling a method on a
// nil *File in Go panics on dereference instead, which isn't a meaningful
// behavior to assert here.
func TestQuotes(t *testing.T) {
	const group = "quote"
	path := filepath.Join("..", "..", "..", "tests", "tst-quote1-data", "quote.conf")

	f, err := econf.ReadFile(path, "=", "#")
	if err != nil {
		t.Fatalf("ReadFile: %v", err)
	}
	defer f.Close()

	tests := []struct {
		key    string
		val    string
		quotes bool
	}{
		{"unquoted", "unquoted", false},
		{"spaces_1", "spaces 1", false},
		{"spaces_2", "  spaces 2  ", true},
		{"mid_quote", `mid"quote`, false},
		{"begin_quote", `"begin_quote`, true},
		{"end_quote", `end quote"`, true},
		{"quoted_word", `a "quoted" word`, true},
		{"quoted_word1", `a 'quoted' word`, true},
		{"quoted_string", `"a quoted string"`, true},
		{"quoted_string1", `'a quoted string'`, true},
		{"unbal1", `"unbalanced`, true},
		{"unbal2", `unbalanced"`, false},
		{"unbal3", `"not "well" balanced`, true},
		{"unbal4", `not "well" balanced"`, false},
		{"quoted_delim", "a=b", true},
		{"quoted_delim1", "=>b", true},
		{"unquoted_delim", "a=b", false},
		{"unquoted_delim1", "=>b", false},
		{"unquoted_delim2", "a=b", false},
		{"unquoted_delim3", "=>b", false},
	}

	for _, tc := range tests {
		got, err := f.String(group, tc.key)
		if err != nil {
			t.Errorf("String(%q): %v", tc.key, err)
			continue
		}
		if got != tc.val {
			t.Errorf("String(%q) = %q, want %q", tc.key, got, tc.val)
		}
		if gotQuotes := f.HasQuotes(group, tc.key); gotQuotes != tc.quotes {
			t.Errorf("HasQuotes(%q) = %v, want %v", tc.key, gotQuotes, tc.quotes)
		}
	}

	if f.HasQuotes(group, "does_not_exist") {
		t.Error("HasQuotes(does_not_exist) = true, want false")
	}

	if err := f.SetQuotes(group, "unquoted", true); err != nil {
		t.Errorf("SetQuotes(unquoted, true): %v", err)
	} else if !f.HasQuotes(group, "unquoted") {
		t.Error("HasQuotes(unquoted) = false, want true after SetQuotes")
	}

	if err := f.SetQuotes(group, "spaces_2", false); err != nil {
		t.Errorf("SetQuotes(spaces_2, false): %v", err)
	} else if f.HasQuotes(group, "spaces_2") {
		t.Error("HasQuotes(spaces_2) = true, want false after SetQuotes")
	}

	if err := f.SetQuotes(group, "does_not_exist", true); !errors.Is(err, econf.ErrNoKey) {
		t.Errorf("SetQuotes(does_not_exist, true) error = %v, want ErrNoKey", err)
	}
}

// TestDefaultQuotesOption replicates tst-quote2.c: the "quotes" flag of a
// newly set entry defaults to false, unless the key file was created with
// the DEFAULT_QUOTES=1 option, and SetQuotes always overrides either
// default. econf_writeFile must honor the resulting flag by enclosing (or
// not enclosing) the written value in quotes.
func TestDefaultQuotesOption(t *testing.T) {
	t.Run("DEFAULT_QUOTES unset", func(t *testing.T) {
		kf, err := econf.NewKeyFileWithOptions("")
		if err != nil {
			t.Fatalf("NewKeyFileWithOptions(\"\"): %v", err)
		}
		defer kf.Close()
		kf.SetDelimiterTag('=')
		kf.SetCommentTag('#')

		if err := kf.SetString("test", "plain", "value1"); err != nil {
			t.Fatalf("SetString(plain): %v", err)
		}
		if kf.HasQuotes("test", "plain") {
			t.Error("HasQuotes(plain) = true, want false by default")
		}

		if err := kf.SetString("test", "forced_quoted", "value2"); err != nil {
			t.Fatalf("SetString(forced_quoted): %v", err)
		}
		if err := kf.SetQuotes("test", "forced_quoted", true); err != nil {
			t.Fatalf("SetQuotes(forced_quoted, true): %v", err)
		}
		if !kf.HasQuotes("test", "forced_quoted") {
			t.Error("HasQuotes(forced_quoted) = false, want true after SetQuotes")
		}

		dir := t.TempDir()
		if err := kf.WriteFile(dir, "out.ini"); err != nil {
			t.Fatalf("WriteFile: %v", err)
		}
		contents, err := os.ReadFile(filepath.Join(dir, "out.ini"))
		if err != nil {
			t.Fatalf("ReadFile(written): %v", err)
		}
		if !strings.Contains(string(contents), "plain=value1") {
			t.Errorf("written file does not contain unquoted %q:\n%s", "plain=value1", contents)
		}
		if !strings.Contains(string(contents), `forced_quoted="value2"`) {
			t.Errorf("written file does not contain quoted %q:\n%s", `forced_quoted="value2"`, contents)
		}
	})

	t.Run("DEFAULT_QUOTES=1", func(t *testing.T) {
		kf, err := econf.NewKeyFileWithOptions("DEFAULT_QUOTES=1")
		if err != nil {
			t.Fatalf("NewKeyFileWithOptions(DEFAULT_QUOTES=1): %v", err)
		}
		defer kf.Close()
		kf.SetDelimiterTag('=')
		kf.SetCommentTag('#')

		if err := kf.SetString("test", "default_quoted", "value3"); err != nil {
			t.Fatalf("SetString(default_quoted): %v", err)
		}
		if !kf.HasQuotes("test", "default_quoted") {
			t.Error("HasQuotes(default_quoted) = false, want true due to DEFAULT_QUOTES=1")
		}

		if err := kf.SetString("test", "forced_unquoted", "value4"); err != nil {
			t.Fatalf("SetString(forced_unquoted): %v", err)
		}
		if err := kf.SetQuotes("test", "forced_unquoted", false); err != nil {
			t.Fatalf("SetQuotes(forced_unquoted, false): %v", err)
		}
		if kf.HasQuotes("test", "forced_unquoted") {
			t.Error("HasQuotes(forced_unquoted) = true, want false after SetQuotes")
		}

		dir := t.TempDir()
		if err := kf.WriteFile(dir, "out.ini"); err != nil {
			t.Fatalf("WriteFile: %v", err)
		}
		contents, err := os.ReadFile(filepath.Join(dir, "out.ini"))
		if err != nil {
			t.Fatalf("ReadFile(written): %v", err)
		}
		if !strings.Contains(string(contents), `default_quoted="value3"`) {
			t.Errorf("written file does not contain quoted %q:\n%s", `default_quoted="value3"`, contents)
		}
		if !strings.Contains(string(contents), "forced_unquoted=value4") {
			t.Errorf("written file does not contain unquoted %q:\n%s", "forced_unquoted=value4", contents)
		}
	})
}
