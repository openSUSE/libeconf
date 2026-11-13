package econf_test

import (
	"errors"
	"path/filepath"
	"testing"

	"github.com/openSUSE/libeconf/go/econf"
)

// TestLoginDefsRealWorldFile replicates tst-logindefs1.c: a real-world
// login.defs, with its "= \t" multi-character delimiter, parses and every
// key is readable.
func TestLoginDefsRealWorldFile(t *testing.T) {
	path := filepath.Join("..", "..", "..", "tests", "tst-logindefs1-data", "etc", "login.defs")
	f, err := econf.ReadFile(path, "= \t", "#")
	if err != nil {
		t.Fatalf("ReadFile: %v", err)
	}
	defer f.Close()

	tests := []struct{ key, want string }{
		{"USERGROUPS_ENAB", "yes"},
		{"ENV_SUPATH", "PATH=/sbin:/bin:/usr/sbin:/usr/bin"},
		{"UMASK", "022"},
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

	keys, err := f.Keys("")
	if err != nil {
		t.Fatalf("Keys: %v", err)
	}
	if len(keys) == 0 {
		t.Fatal("Keys() returned no keys")
	}
}

// TestLoginDefsValueParsing replicates tst-logindefs2.c: a util-linux style
// logindefs.data exercises the "= \t" delimiter against quoted strings,
// inline comments, "KEY = value", "KEY=value" and "KEY  value" spellings,
// octal numbers, booleans, a value-conversion error on a non-numeric
// string, and StringDef/BoolDef fallbacks.
//
// EMPTY has no value in the file. The upstream C test asserts that reading
// it yields a NULL string; Go's String() has no NULL/"" distinction (see
// the same note for LINE4 in getconfdirs_test.go), so here it is asserted
// to be an empty string with no error instead.
func TestLoginDefsValueParsing(t *testing.T) {
	path := filepath.Join("..", "..", "..", "tests", "tst-logindefs2-data", "logindefs.data")
	f, err := econf.ReadFile(path, "= \t", "#")
	if err != nil {
		t.Fatalf("ReadFile: %v", err)
	}
	defer f.Close()

	keys, err := f.Keys("")
	if err != nil {
		t.Fatalf("Keys: %v", err)
	}
	if len(keys) == 0 {
		t.Fatal("Keys() returned no keys")
	}

	if got, err := f.String("", "STRING"); err != nil || got != "this_is_string" {
		t.Errorf("String(STRING) = %q, %v, want \"this_is_string\"", got, err)
	}

	if got, err := f.Int("", "NUMBER"); err != nil || got != 123456 {
		t.Errorf("Int(NUMBER) = %d, %v, want 123456", got, err)
	}
	if _, err := f.Int("", "CRAZY1"); !errors.Is(err, econf.ErrValueConversionError) {
		t.Errorf("Int(CRAZY1) error = %v, want ErrValueConversionError", err)
	}

	if got, err := f.UInt64("", "OKTAL"); err != nil || got != 400 {
		t.Errorf("UInt64(OKTAL) = %d, %v, want 400 (octal 0620)", got, err)
	}
	if _, err := f.UInt64("", "CRAZY1"); !errors.Is(err, econf.ErrValueConversionError) {
		t.Errorf("UInt64(CRAZY1) error = %v, want ErrValueConversionError", err)
	}

	if got, err := f.Bool("", "BOOLEAN"); err != nil || got != true {
		t.Errorf("Bool(BOOLEAN) = %v, %v, want true", got, err)
	}

	stringTests := []struct{ key, want string }{
		{"CRAZY1", "this is crazy format"},
		{"CRAZY2", "fooBar"},
		{"CRAZY3", "FoooBaaar"},
		{"EMPTY", ""},
		{"END", "the is end"},
	}
	for _, tc := range stringTests {
		got, err := f.String("", tc.key)
		if err != nil {
			t.Errorf("String(%q): %v", tc.key, err)
			continue
		}
		if got != tc.want {
			t.Errorf("String(%q) = %q, want %q", tc.key, got, tc.want)
		}
	}

	if _, err := f.String("", "NOTEXIST"); !errors.Is(err, econf.ErrNoKey) {
		t.Errorf("String(NOTEXIST) error = %v, want ErrNoKey", err)
	}

	if got, err := f.StringDef("", "END", "DEF"); err != nil || got != "the is end" {
		t.Errorf("StringDef(END, DEF) = %q, %v, want \"the is end\", nil", got, err)
	}
	if got, err := f.StringDef("", "NOTEXIST", "DEF"); !errors.Is(err, econf.ErrNoKey) || got != "DEF" {
		t.Errorf("StringDef(NOTEXIST, DEF) = %q, %v, want \"DEF\", ErrNoKey", got, err)
	}

	if got, err := f.BoolDef("", "BOOLEAN", false); err != nil || got != true {
		t.Errorf("BoolDef(BOOLEAN, false) = %v, %v, want true, nil", got, err)
	}
	if got, err := f.BoolDef("", "NOTEXIST", false); !errors.Is(err, econf.ErrNoKey) || got != false {
		t.Errorf("BoolDef(NOTEXIST, false) = %v, %v, want false, ErrNoKey", got, err)
	}
}
