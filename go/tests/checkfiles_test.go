package econf_test

import (
	"errors"
	"os"
	"path/filepath"
	"testing"

	"github.com/openSUSE/libeconf/go/econf"
)

// TestReadFileWithCallbackRejectsSymlink replicates tst-checkfiles.c: a
// user-supplied callback can reject a candidate file (here, because it's a
// symlink), aborting the read with ErrParsingCallbackFailed.
//
// The C callback takes an extra void *data parameter to carry the "test"
// string alongside the filename; Go's FileCallback is a plain closure (see
// econf_go_FileCallback(3)), so that string is simply captured directly
// instead of threaded through callback_data.
func TestReadFileWithCallbackRejectsSymlink(t *testing.T) {
	target, err := filepath.Abs(filepath.Join("..", "..", "tests", "tst-arguments-string", "etc", "arguments.conf"))
	if err != nil {
		t.Fatalf("filepath.Abs: %v", err)
	}

	dir := t.TempDir()
	link := filepath.Join(dir, "link.conf")
	if err := os.Symlink(target, link); err != nil {
		t.Fatalf("Symlink: %v", err)
	}

	extra := "test"
	rejectSymlinks := func(filename string) bool {
		t.Logf("checking %s (extra: %s)", filename, extra)
		info, err := os.Lstat(filename)
		if err != nil || info.Mode()&os.ModeSymlink != 0 {
			return false
		}
		return true
	}

	_, err = econf.ReadFileWithCallback(link, "=", "#", rejectSymlinks)
	if !errors.Is(err, econf.ErrParsingCallbackFailed) {
		t.Errorf("ReadFileWithCallback(symlink) error = %v, want ErrParsingCallbackFailed", err)
	}
}
