package econf_test

import (
	"path/filepath"
	"testing"

	"github.com/openSUSE/libeconf/go/econf"
)

// TestPath replicates tst-getpath.c: a file produced by merging several
// files together has no path of its own (Path() returns ""), while a file
// read directly from disk reports its absolute path.
//
// econf_readDirs itself isn't exposed by this package (see the same
// note in getconfdirs_test.go), so the merged-file half uses
// ReadDirsHistory + the mergeHistory helper from uapi_test.go instead.
func TestPath(t *testing.T) {
	t.Run("merged file has no path", func(t *testing.T) {
		base := filepath.Join("..", "..", "tests", "tst-getconfdirs1-data")
		files, err := econf.ReadDirsHistory(filepath.Join(base, "usr", "etc"), filepath.Join(base, "etc"), "getconfdir", "conf", "=", "#")
		if err != nil {
			t.Fatalf("ReadDirsHistory: %v", err)
		}
		merged := mergeHistory(t, files)

		if got := merged.Path(); got != "" {
			t.Errorf("Path() = %q, want \"\"", got)
		}
	})

	t.Run("directly read file has its absolute path", func(t *testing.T) {
		relPath := filepath.Join("..", "..", "tests", "tst-merge1-data", "usr", "etc", "tst-merge1.conf")
		wantPath, err := filepath.Abs(relPath)
		if err != nil {
			t.Fatalf("filepath.Abs: %v", err)
		}

		f, err := econf.ReadFile(relPath, "=", "#")
		if err != nil {
			t.Fatalf("ReadFile: %v", err)
		}
		defer f.Close()

		if got := f.Path(); got != wantPath {
			t.Errorf("Path() = %q, want %q", got, wantPath)
		}
	})
}
