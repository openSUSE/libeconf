package econf_test

import (
	"path/filepath"
	"testing"

	"github.com/openSUSE/libeconf/go/econf"
)

// mergeHistory mirrors the package-internal mergeFileList algorithm using
// only exported API: files are merged in order, except that a file is
// skipped entirely if a later (higher priority) file in the list shares
// its basename. ReadConfig does this internally; ReadDirsHistory leaves it
// to the caller.
func mergeHistory(t *testing.T, files []*econf.File) *econf.File {
	t.Helper()
	if len(files) == 0 {
		t.Fatal("mergeHistory: no files")
	}
	merged := files[0]
	for i := 1; i < len(files); i++ {
		current := files[i]
		currentBase := filepath.Base(current.Path())

		superseded := false
		for j := i + 1; j < len(files); j++ {
			if filepath.Base(files[j].Path()) == currentBase {
				superseded = true
				break
			}
		}
		if superseded {
			continue
		}

		var err error
		merged, err = econf.MergeFiles(merged, current)
		if err != nil {
			t.Fatalf("MergeFiles: %v", err)
		}
	}
	return merged
}

// TestUAPIFixtures replicates a subset of tests/tst-uapi-*.c against the
// public ReadDirsHistory/MergeFiles API, using the same fixture directories
// and expected results as the upstream C tests. econf_readConfig's
// uapi-1/uapi-2 scenarios (which need a third, /run, search directory) are
// not covered here since ReadDirsHistory only supports two; the C tests
// also only reach those fixture directories via the ROOT_PREFIX option,
// which isn't part of this package's public surface.
func TestUAPIFixtures(t *testing.T) {
	const base = "../../tests"

	tests := []struct {
		name         string
		dataDir      string
		usrConfDir   string
		etcConfDir   string
		configName   string
		configSuffix string
		want         map[string]*string
	}{
		{
			name: "uapi-5", dataDir: "tst-uapi-5-data",
			usrConfDir: "usr/etc/foo", etcConfDir: "etc/foo",
			configName: "bar", configSuffix: "conf",
			want: map[string]*string{"ETC": ptr("true"), "KEY1": ptr("b"), "KEY2": ptr("b"), "KEY3": ptr("a")},
		},
		{
			name: "uapi-6", dataDir: "tst-uapi-6-data",
			usrConfDir: "usr/lib/foo", etcConfDir: "etc/foo",
			configName: "bar", configSuffix: "conf",
			want: map[string]*string{"ETC": ptr("false"), "KEY1": ptr("a"), "KEY2": ptr("a")},
		},
		{
			name: "uapi-7", dataDir: "tst-uapi-7-data",
			usrConfDir: "usr/lib/foo", etcConfDir: "etc/foo",
			configName: "bar", configSuffix: "conf",
			want: map[string]*string{"ETC": ptr("false"), "KEY1": nil, "KEY2": ptr("a")},
		},
		{
			// uapi-9 calls econf_readConfig with an empty config_name,
			// which makes it treat "foo" as the config name itself and
			// search "<dir>/foo.d/*" (not "<dir>/foo.conf.d/*"); passing
			// an empty configSuffix here reproduces that drop-in-without-
			// main-config-file layout via ReadDirsHistory.
			name: "uapi-9", dataDir: "tst-uapi-9-data",
			usrConfDir: "usr/lib", etcConfDir: "etc",
			configName: "foo", configSuffix: "",
			want: map[string]*string{"KEY1": ptr("b"), "KEY2": ptr("b"), "KEY3": ptr("a"), "ETC": ptr("true")},
		},
		{
			name: "uapi-10", dataDir: "tst-uapi-10-data",
			usrConfDir: "usr/etc", etcConfDir: "etc",
			configName: "bar", configSuffix: "conf",
			want: map[string]*string{"ETC": ptr("true"), "KEY1": ptr("b"), "KEY2": ptr("b"), "KEY3": ptr("a")},
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			root := filepath.Join(base, tc.dataDir)
			usrConfDir := filepath.Join(root, tc.usrConfDir)
			etcConfDir := filepath.Join(root, tc.etcConfDir)

			files, err := econf.ReadDirsHistory(usrConfDir, etcConfDir, tc.configName, tc.configSuffix, "=", "#")
			if err != nil {
				t.Fatalf("ReadDirsHistory: %v", err)
			}
			merged := mergeHistory(t, files)

			for key, want := range tc.want {
				got, err := merged.String("", key)
				if want == nil {
					if err == nil {
						t.Errorf("%s: want no value, got %q", key, got)
					}
					continue
				}
				if err != nil || got != *want {
					t.Errorf("%s = %q, %v; want %q", key, got, err, *want)
				}
			}
		})
	}
}

func ptr(s string) *string { return &s }
