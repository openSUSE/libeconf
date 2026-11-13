package econf_test

import (
	"path/filepath"
	"testing"

	"github.com/openSUSE/libeconf/go/econf"
)

// TestManualParsingDirs replicates tst-man-dir.c: PARSING_DIRS=usr:run:etc,
// set via NewKeyFileWithOptions, makes (*File).ReadConfig search exactly
// those three directories (in that order, highest priority last) instead
// of the directories it would otherwise derive from usrSubdir/project, so
// /run and /usr/etc are both present in the search but /etc's value wins.
func TestManualParsingDirs(t *testing.T) {
	base, err := filepath.Abs(filepath.Join("..", "..", "tests", "tst-man-dir-data"))
	if err != nil {
		t.Fatalf("filepath.Abs: %v", err)
	}

	f, err := econf.NewKeyFileWithOptions(
		"PARSING_DIRS=" + filepath.Join(base, "usr", "etc") + ":" + filepath.Join(base, "run") + ":" + filepath.Join(base, "etc"))
	if err != nil {
		t.Fatalf("NewKeyFileWithOptions: %v", err)
	}
	defer f.Close()

	if err := f.ReadConfig("", filepath.Join(base, "usr", "etc"), "getconfdir", "conf", "=", "#"); err != nil {
		t.Fatalf("ReadConfig: %v", err)
	}

	checkConfDirKey(t, f, "KEY1", ptr("etc"))
	checkConfDirKey(t, f, "USRETC", nil)
	checkConfDirKey(t, f, "RUN", nil)
	checkConfDirKey(t, f, "ETC", ptr("true"))
}
