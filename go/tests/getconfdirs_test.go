package econf_test

import (
	"path/filepath"
	"testing"

	"github.com/openSUSE/libeconf/go/econf"
)

// readDirsMerged replicates the deprecated econf_readDirs: it evaluates the
// "main" config file plus every *.d drop-in across usrConfDir and
// etcConfDir (via ReadDirsHistory) and merges them into a single result,
// highest-priority (etc) last. Neither Go package exposes econf_readDirs
// itself (see go/native/README.md: "deprecated ... functions" are
// deliberately not implemented), so tests that used it are ported against
// the public ReadDirsHistory + MergeFiles combination instead, using the
// same merge order (mergeHistory in uapi_test.go) that
// econf_readConfig/econf_readDirs use internally.
func readDirsMerged(t *testing.T, usrConfDir, etcConfDir, configName, configSuffix string) *econf.File {
	t.Helper()
	files, err := econf.ReadDirsHistory(usrConfDir, etcConfDir, configName, configSuffix, "=", "#")
	if err != nil {
		t.Fatalf("ReadDirsHistory: %v", err)
	}
	return mergeHistory(t, files)
}

func checkConfDirKey(t *testing.T, f *econf.File, key string, want *string) {
	t.Helper()
	got, err := f.String("", key)
	if want == nil {
		if err == nil {
			t.Errorf("%s: want no value, got %q", key, got)
		}
		return
	}
	if err != nil || got != *want {
		t.Errorf("%s = %q, %v; want %q", key, got, err, *want)
	}
}

// TestGetConfDirsEtcOverridesUsr replicates tst-getconfdirs1.c: when both
// /usr/etc/getconfdir.conf and /etc/getconfdir.conf exist, the /etc main
// file wins outright (the /usr/etc one is ignored), and /etc's *.conf.d
// drop-ins (but not the suffix-less "9-broken" file) are layered on top.
func TestGetConfDirsEtcOverridesUsr(t *testing.T) {
	base := filepath.Join("..", "..", "..", "tests", "tst-getconfdirs1-data")
	merged := readDirsMerged(t, filepath.Join(base, "usr", "etc"), filepath.Join(base, "etc"), "getconfdir", ".conf")

	checkConfDirKey(t, merged, "KEY1", ptr("etcconfd"))
	checkConfDirKey(t, merged, "USRETC", nil)
	checkConfDirKey(t, merged, "ETC", ptr("true"))
	checkConfDirKey(t, merged, "OVERRIDE", ptr("true"))
}

// TestGetConfDirsUsrOnly replicates tst-getconfdirs3.c: only
// /usr/etc/getconfdir.conf exists, so only its values are present.
func TestGetConfDirsUsrOnly(t *testing.T) {
	base := filepath.Join("..", "..", "..", "tests", "tst-getconfdirs3-data")
	merged := readDirsMerged(t, filepath.Join(base, "usr", "etc"), filepath.Join(base, "etc"), "getconfdir", ".conf")

	checkConfDirKey(t, merged, "KEY1", ptr("usretc"))
	checkConfDirKey(t, merged, "USRETC", ptr("true"))
	checkConfDirKey(t, merged, "ETC", nil)
	checkConfDirKey(t, merged, "OVERRIDE", nil)
}

// TestGetConfDirsEtcOnly replicates tst-getconfdirs4.c: only
// /etc/getconfdir.conf exists, so only its values are present. This eases
// migration when the same config file is read by different projects.
func TestGetConfDirsEtcOnly(t *testing.T) {
	base := filepath.Join("..", "..", "..", "tests", "tst-getconfdirs4-data")
	merged := readDirsMerged(t, filepath.Join(base, "usr", "etc"), filepath.Join(base, "etc"), "getconfdir", ".conf")

	checkConfDirKey(t, merged, "KEY1", ptr("etc"))
	checkConfDirKey(t, merged, "ETC", ptr("true"))
	checkConfDirKey(t, merged, "USRETC", nil)
	checkConfDirKey(t, merged, "OVERRIDE", nil)
}

// TestGetConfDirsDropInOnly replicates tst-getconfdirs5.c: neither a
// /usr/etc/sysctl.conf nor an /etc/sysctl.conf main file exists, only
// *.d drop-ins under both sysctl.conf.d directories, with /etc winning on
// conflicts.
func TestGetConfDirsDropInOnly(t *testing.T) {
	base := filepath.Join("..", "..", "..", "tests", "tst-getconfdirs5-data")
	merged := readDirsMerged(t, filepath.Join(base, "usr", "etc"), filepath.Join(base, "etc"), "sysctl", "conf")

	checkConfDirKey(t, merged, "KEY1", ptr("etc"))
	checkConfDirKey(t, merged, "ETC", ptr("true"))
	checkConfDirKey(t, merged, "USRETC", ptr("true"))
	checkConfDirKey(t, merged, "OVERRIDE", nil)
}

// TestGetConfDirsUsrDropInNotIgnored replicates tst-getconfdirs6.c: when
// /usr/etc/getconfdir.ini.d has a drop-in but /etc/getconfdir.ini (the main
// file) does not exist, the /usr/etc drop-in must still be read, not
// ignored, and /etc's own *.ini.d drop-ins still layer on top of it.
func TestGetConfDirsUsrDropInNotIgnored(t *testing.T) {
	base := filepath.Join("..", "..", "..", "tests", "tst-getconfdirs6-data")
	merged := readDirsMerged(t, filepath.Join(base, "usr", "etc"), filepath.Join(base, "etc"), "getconfdir", "ini")

	checkConfDirKey(t, merged, "KEY1", ptr("etcinid"))
	checkConfDirKey(t, merged, "USRETC", ptr("true"))
	checkConfDirKey(t, merged, "ETC", nil)
	checkConfDirKey(t, merged, "OVERRIDE", ptr("true"))
}

// TestGetConfDirsOverrideReplacesEmptyValue replicates tst-getconfdirs7.c:
// /usr/etc ships a main file with some placeholder-empty values, and an
// /etc *.d drop-in replaces most (but not all) of them with real values.
//
// LINE4 is left at its placeholder-empty value by both files. The upstream
// C test asserts that reading it then yields a NULL string, distinct from
// a successful empty string; Go's String() has no such NULL/"" distinction
// (see also the "NULL means empty string" handling in TestGroupsSetAndGetRoundTrip),
// so here it is asserted to be an empty string with no error instead.
func TestGetConfDirsOverrideReplacesEmptyValue(t *testing.T) {
	base := filepath.Join("..", "..", "..", "tests", "tst-getconfdirs7-data")
	merged := readDirsMerged(t, filepath.Join(base, "usr", "etc"), filepath.Join(base, "etc"), "lcdnetmon", "conf")

	checkConfDirKey(t, merged, "BUSID", ptr("1"))
	checkConfDirKey(t, merged, "ADDRESS", ptr("0x27"))
	checkConfDirKey(t, merged, "ROWS", ptr("4"))
	checkConfDirKey(t, merged, "LINE1", ptr("eth0/eth1"))
	checkConfDirKey(t, merged, "LINE2", ptr(`\4{eth0}`))
	checkConfDirKey(t, merged, "LINE3", ptr(`\4{eth1}`))
	checkConfDirKey(t, merged, "LINE4", ptr(""))
}

// TestGetConfDirsOneDirMissing replicates tst-getconfdirs9.c: passing ""
// for one of usrConfDir/etcConfDir (Go's equivalent of the C API's NULL)
// must behave exactly like that directory not existing, in both
// directions.
func TestGetConfDirsOneDirMissing(t *testing.T) {
	etcDir := filepath.Join("..", "..", "..", "tests", "tst-getconfdirs1-data", "etc")
	usrDir := filepath.Join("..", "..", "..", "tests", "tst-getconfdirs1-data", "usr", "etc")

	t.Run("usr-unset", func(t *testing.T) {
		merged := readDirsMerged(t, "", etcDir, "getconfdir", ".conf")
		checkConfDirKey(t, merged, "KEY1", ptr("etcconfd"))
		checkConfDirKey(t, merged, "USRETC", nil)
		checkConfDirKey(t, merged, "ETC", ptr("true"))
		checkConfDirKey(t, merged, "OVERRIDE", ptr("true"))
	})

	t.Run("etc-unset", func(t *testing.T) {
		merged := readDirsMerged(t, usrDir, "", "getconfdir", ".conf")
		checkConfDirKey(t, merged, "KEY1", ptr("usretc"))
		checkConfDirKey(t, merged, "USRETC", ptr("true"))
		checkConfDirKey(t, merged, "ETC", nil)
		checkConfDirKey(t, merged, "OVERRIDE", nil)
	})
}

// TestGetConfDirsCustomConfigAndParsingDirs replicates the first half of
// tst-getconfdirs8.c: CONFIG_DIRS=.conf.d:.d and PARSING_DIRS=<usr>:<etc>,
// set via NewKeyFileWithOptions, make (*File).ReadConfig search both the
// usr and etc directories given in PARSING_DIRS (ignoring project/
// usrSubdir) and scan both of CONFIG_DIRS's drop-in suffixes
// (".conf.d" and ".d") - so both /etc/getconfdir.d (the plain ".d"
// drop-in) and the ".conf.d" drop-ins under both directories are found.
//
// Not ported: the second half of tst-getconfdirs8.c, which repeats the
// same scenario via the global econf_set_conf_dirs setter. That function
// is deliberately not implemented in either Go package (see
// go/native/README.md).
func TestGetConfDirsCustomConfigAndParsingDirs(t *testing.T) {
	base, err := filepath.Abs(filepath.Join("..", "..", "..", "tests", "tst-getconfdirs8-data"))
	if err != nil {
		t.Fatalf("filepath.Abs: %v", err)
	}

	f, err := econf.NewKeyFileWithOptions(
		"CONFIG_DIRS=.conf.d:.d;PARSING_DIRS=" + filepath.Join(base, "usr", "etc") + ":" + filepath.Join(base, "etc"))
	if err != nil {
		t.Fatalf("NewKeyFileWithOptions: %v", err)
	}
	defer f.Close()

	if err := f.ReadConfig("", "", "getconfdir", "conf", "=", "#"); err != nil {
		t.Fatalf("ReadConfig: %v", err)
	}

	checkConfDirKey(t, f, "KEY1", ptr("etcconfd"))
	checkConfDirKey(t, f, "KEY2", ptr("usrconfd"))
	checkConfDirKey(t, f, "KEY3", ptr("etcconfd3"))
	checkConfDirKey(t, f, "USRETC", nil)
	checkConfDirKey(t, f, "ETC", ptr("true"))
	checkConfDirKey(t, f, "KEY4", ptr("etcd4"))
}
