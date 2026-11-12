package econf_test

import (
	"errors"
	"path/filepath"
	"testing"

	"github.com/openSUSE/libeconf/go/econf"
)

// TestShellsNoDelimiterFile replicates the portable parts of
// tst-shells1.c: /etc/shells has no delimiter, so each (comment-stripped)
// line becomes a key with an empty value, and the file round-trips through
// WriteFile/ReadFile.
//
// Not ported: the econf_getExtValue checks for the comment attached to
// "/bin/bash" and "/bin/csh". That extended API is deliberately not
// exposed by either Go package (see go/native/README.md), matching the
// skip of tst-comments.c in comments_test.go.
func TestShellsNoDelimiterFile(t *testing.T) {
	path := filepath.Join("..", "..", "..", "tests", "tst-shells1-data", "etc", "shells")
	f, err := econf.ReadFile(path, "", "#")
	if err != nil {
		t.Fatalf("ReadFile: %v", err)
	}
	defer f.Close()

	keys, err := f.Keys("")
	if err != nil {
		t.Fatalf("Keys: %v", err)
	}
	if len(keys) != 24 {
		t.Fatalf("Keys() returned %d keys, want 24: %v", len(keys), keys)
	}

	if got, err := f.String("", "/usr/bin/bash"); err != nil || got != "" {
		t.Fatalf("String(/usr/bin/bash) = %q, %v, want \"\", nil", got, err)
	}

	if _, err := f.String("", "doesnotexist"); !errors.Is(err, econf.ErrNoKey) {
		t.Fatalf("String(doesnotexist) error = %v, want ErrNoKey", err)
	}

	dir := t.TempDir()
	if err := f.WriteFile(dir, "out.ini"); err != nil {
		t.Fatalf("WriteFile: %v", err)
	}
	roundTripped, err := econf.ReadFile(filepath.Join(dir, "out.ini"), "", "#")
	if err != nil {
		t.Fatalf("ReadFile(roundtrip): %v", err)
	}
	defer roundTripped.Close()
}

// TestShellsDropInMaskedByDevNull replicates tst-shells2.c: /usr/etc/shells
// is the main file, /usr/etc/shells.d/tcsh adds "/bin/csh" and
// "/bin/tcsh", but /etc/shells.d/tcsh (a same-named, higher-priority
// drop-in with different content) entirely supersedes it rather than
// merging with it, so "/bin/csh" ends up absent even though a drop-in once
// granted it. /etc/shells.d/foo is a symlink to /dev/null, the standard
// "mask this vendor drop-in" trick, which likewise entirely supersedes
// (and empties out) /usr/etc/shells.d/foo's "/bin/foo" entry.
//
// econf_readDirs itself isn't exposed by either Go package (see the same
// note in getconfdirs_test.go), so this uses ReadDirsHistory + the
// mergeHistory helper from uapi_test.go instead.
//
// The directories are resolved to absolute paths (matching the upstream
// test's use of the absolute TESTSDIR prefix): getAbsolutePath only runs
// relative paths through filepath.EvalSymlinks, so a relative path here
// would resolve the /dev/null symlink away, changing its basename from
// "foo" to "null" and silently breaking the masking behavior under test.
func TestShellsDropInMaskedByDevNull(t *testing.T) {
	base, err := filepath.Abs(filepath.Join("..", "..", "..", "tests", "tst-shells2-data"))
	if err != nil {
		t.Fatalf("filepath.Abs: %v", err)
	}
	files, err := econf.ReadDirsHistory(filepath.Join(base, "usr", "etc"), filepath.Join(base, "etc"), "shells", "", "", "#")
	if err != nil {
		t.Fatalf("ReadDirsHistory: %v", err)
	}
	merged := mergeHistory(t, files)

	keys, err := merged.Keys("")
	if err != nil {
		t.Fatalf("Keys: %v", err)
	}
	if len(keys) == 0 {
		t.Fatal("Keys() returned no keys")
	}

	tests := []struct {
		shell    string
		expected bool
	}{
		{"/bin/csh", false},
		{"/bin/tcsh", true},
		{"/bin/foo", false},
	}
	for _, tc := range tests {
		_, err := merged.String("", tc.shell)
		found := err == nil
		if found != tc.expected {
			t.Errorf("String(%q): found=%v, want found=%v (err=%v)", tc.shell, found, tc.expected, err)
		}
	}
}
