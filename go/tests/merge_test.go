package econf_test

import (
	"errors"
	"path/filepath"
	"testing"

	"github.com/openSUSE/libeconf/go/econf"
)

// checkMergedString mirrors the check_string helper repeated across
// tst-merge4.c, tst-merge5.c and tst-merge6.c.
func checkMergedString(t *testing.T, f *econf.File, group, key, want string) {
	t.Helper()
	got, err := f.String(group, key)
	if err != nil {
		t.Fatalf("String(%q, %q): %v", group, key, err)
	}
	if got != want {
		t.Fatalf("String(%q, %q) = %q, want %q", group, key, got, want)
	}
}

// TestMergeFilesWithOneMissingFile replicates tst-merge1.c and
// tst-merge2.c: merging a file that was successfully read with one that
// failed to read (ReadFile returns a nil *File) must not crash, and must
// fail with ErrArgumentIsNullValue rather than silently producing a result.
func TestMergeFilesWithOneMissingFile(t *testing.T) {
	t.Run("usr-missing", func(t *testing.T) {
		// tst-merge1.c: /etc has no config file, /usr/etc does.
		etcPath := filepath.Join("..", "..", "..", "tests", "tst-merge1-data", "etc", "tst-merge1.conf")
		usrPath := filepath.Join("..", "..", "..", "tests", "tst-merge1-data", "usr", "etc", "tst-merge1.conf")

		etcFile, err := econf.ReadFile(etcPath, "=", "#")
		if !errors.Is(err, econf.ErrNoFile) || etcFile != nil {
			t.Fatalf("ReadFile(etc) = %v, %v; want nil, ErrNoFile", etcFile, err)
		}
		usrFile, err := econf.ReadFile(usrPath, "=", "#")
		if err != nil || usrFile == nil {
			t.Fatalf("ReadFile(usr) = %v, %v; want a file, no error", usrFile, err)
		}
		defer usrFile.Close()

		merged, err := econf.MergeFiles(usrFile, etcFile)
		if !errors.Is(err, econf.ErrArgumentIsNullValue) || merged != nil {
			t.Fatalf("MergeFiles(usr, nil) = %v, %v; want nil, ErrArgumentIsNullValue", merged, err)
		}
	})

	t.Run("etc-missing", func(t *testing.T) {
		// tst-merge2.c: /etc has a config file, /usr/etc does not.
		etcPath := filepath.Join("..", "..", "..", "tests", "tst-merge2-data", "etc", "tst-merge2.conf")
		usrPath := filepath.Join("..", "..", "..", "tests", "tst-merge2-data", "usr", "etc", "tst-merge2.conf")

		etcFile, err := econf.ReadFile(etcPath, "=", "#")
		if err != nil || etcFile == nil {
			t.Fatalf("ReadFile(etc) = %v, %v; want a file, no error", etcFile, err)
		}
		defer etcFile.Close()
		usrFile, err := econf.ReadFile(usrPath, "=", "#")
		if !errors.Is(err, econf.ErrNoFile) || usrFile != nil {
			t.Fatalf("ReadFile(usr) = %v, %v; want nil, ErrNoFile", usrFile, err)
		}

		merged, err := econf.MergeFiles(usrFile, etcFile)
		if !errors.Is(err, econf.ErrArgumentIsNullValue) || merged != nil {
			t.Fatalf("MergeFiles(nil, etc) = %v, %v; want nil, ErrArgumentIsNullValue", merged, err)
		}
	})
}

// TestMergeFilesCommentOnlyFile replicates tst-merge3.c: merging a real,
// fully populated file with one that parses to zero key/value entries
// (/etc/default/su contains only comments) must not crash and must produce
// a usable merged result.
func TestMergeFilesCommentOnlyFile(t *testing.T) {
	loginDefsPath := filepath.Join("..", "..", "..", "tests", "tst-merge3-data", "etc", "login.defs")
	suPath := filepath.Join("..", "..", "..", "tests", "tst-merge3-data", "etc", "default", "su")

	etcFile, err := econf.ReadFile(loginDefsPath, " \t=", "#")
	if err != nil || etcFile == nil {
		t.Fatalf("ReadFile(login.defs) = %v, %v; want a file, no error", etcFile, err)
	}
	defer etcFile.Close()
	usrFile, err := econf.ReadFile(suPath, " \t=", "#")
	if err != nil || usrFile == nil {
		t.Fatalf("ReadFile(su) = %v, %v; want a file, no error", usrFile, err)
	}
	defer usrFile.Close()

	merged, err := econf.MergeFiles(usrFile, etcFile)
	if err != nil || merged == nil {
		t.Fatalf("MergeFiles = %v, %v; want a file, no error", merged, err)
	}
	defer merged.Close()

	checkMergedString(t, merged, "", "FAIL_DELAY", "3")
}

// TestMergeFilesGroupedWithUngrouped replicates tst-merge4.c and
// tst-merge5.c: merging a file that only has grouped keys with one that
// only has an ungrouped (root) key must keep both without one clobbering
// the other, regardless of which side is "usr" and which is "etc".
func TestMergeFilesGroupedWithUngrouped(t *testing.T) {
	t.Run("ungrouped-is-usr", func(t *testing.T) {
		// tst-merge4.c: data1.conf (etc) has [global]/[default] groups;
		// data2.conf (usr) has a single ungrouped KEY1.
		grouped, err := econf.ReadFile(filepath.Join("..", "..", "..", "tests", "tst-merge4-data", "data1.conf"), "=", "#")
		if err != nil || grouped == nil {
			t.Fatalf("ReadFile(data1.conf) = %v, %v; want a file, no error", grouped, err)
		}
		defer grouped.Close()
		ungrouped, err := econf.ReadFile(filepath.Join("..", "..", "..", "tests", "tst-merge4-data", "data2.conf"), " \t=", "#")
		if err != nil || ungrouped == nil {
			t.Fatalf("ReadFile(data2.conf) = %v, %v; want a file, no error", ungrouped, err)
		}
		defer ungrouped.Close()

		merged, err := econf.MergeFiles(ungrouped, grouped)
		if err != nil || merged == nil {
			t.Fatalf("MergeFiles = %v, %v; want a file, no error", merged, err)
		}
		defer merged.Close()

		checkMergedString(t, merged, "global", "KEY1", "global")
		checkMergedString(t, merged, "default", "KEY1", "default")
		checkMergedString(t, merged, "", "KEY1", "data2")
	})

	t.Run("ungrouped-is-etc", func(t *testing.T) {
		// tst-merge5.c: data1.conf (etc) has a single ungrouped KEY1;
		// data2.conf (usr) has [global]/[default] groups.
		ungrouped, err := econf.ReadFile(filepath.Join("..", "..", "..", "tests", "tst-merge5-data", "data1.conf"), "=", "#")
		if err != nil || ungrouped == nil {
			t.Fatalf("ReadFile(data1.conf) = %v, %v; want a file, no error", ungrouped, err)
		}
		defer ungrouped.Close()
		grouped, err := econf.ReadFile(filepath.Join("..", "..", "..", "tests", "tst-merge5-data", "data2.conf"), " \t=", "#")
		if err != nil || grouped == nil {
			t.Fatalf("ReadFile(data2.conf) = %v, %v; want a file, no error", grouped, err)
		}
		defer grouped.Close()

		merged, err := econf.MergeFiles(grouped, ungrouped)
		if err != nil || merged == nil {
			t.Fatalf("MergeFiles = %v, %v; want a file, no error", merged, err)
		}
		defer merged.Close()

		checkMergedString(t, merged, "global", "KEY1", "global")
		checkMergedString(t, merged, "default", "KEY1", "default")
		checkMergedString(t, merged, "", "KEY1", "data1")
	})
}

// TestMergeFilesAllKeysCommentedInGroup replicates tst-merge6.c: a
// regression test for a crash (sd-report-collector) where merging a "usr"
// file whose only group has every key commented out (zero active entries,
// but the group name is still known) with an "etc" file that adds a real
// key to that same group segfaulted in merge_existing_groups() due to a
// size_t underflow when indexing into the usr file's empty entries.
func TestMergeFilesAllKeysCommentedInGroup(t *testing.T) {
	usrFile, err := econf.ReadFile(filepath.Join("..", "..", "..", "tests", "tst-merge6-data", "data1.conf"), "=", "#")
	if err != nil || usrFile == nil {
		t.Fatalf("ReadFile(data1.conf) = %v, %v; want a file, no error", usrFile, err)
	}
	defer usrFile.Close()
	etcFile, err := econf.ReadFile(filepath.Join("..", "..", "..", "tests", "tst-merge6-data", "data2.conf"), "=", "#")
	if err != nil || etcFile == nil {
		t.Fatalf("ReadFile(data2.conf) = %v, %v; want a file, no error", etcFile, err)
	}
	defer etcFile.Close()

	merged, err := econf.MergeFiles(usrFile, etcFile)
	if err != nil || merged == nil {
		t.Fatalf("MergeFiles = %v, %v; want a file, no error", merged, err)
	}
	defer merged.Close()

	checkMergedString(t, merged, "Server", "ListenAddress", "127.0.0.1:9443")
}
