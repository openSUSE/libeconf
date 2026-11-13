package econf_test

import (
	"path/filepath"
	"testing"

	"github.com/openSUSE/libeconf/go/econf"
)

// TestReadDirsWithoutSuffix replicates tst-without-suffix.c: reading an
// os-release-style file from /usr/lib and /etc with no configured suffix
// (an empty configSuffix, Go's equivalent of passing NULL in C) still
// finds and merges both files, with /etc winning.
//
// econf_readDirs itself isn't exposed by this package (see the same
// note in getconfdirs_test.go), so this uses ReadDirsHistory + the
// mergeHistory helper from uapi_test.go instead.
func TestReadDirsWithoutSuffix(t *testing.T) {
	base := filepath.Join("..", "..", "tests", "tst-without-suffix")
	files, err := econf.ReadDirsHistory(filepath.Join(base, "usr", "lib"), filepath.Join(base, "etc"), "os-release", "", "=", "#")
	if err != nil {
		t.Fatalf("ReadDirsHistory: %v", err)
	}
	merged := mergeHistory(t, files)

	got, err := merged.String("", "VERSION")
	if err != nil {
		t.Fatalf("String(VERSION): %v", err)
	}
	if got != "15.2" {
		t.Errorf("String(VERSION) = %q, want \"15.2\" (the /etc value)", got)
	}
}
