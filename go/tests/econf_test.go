package econf_test

import (
	"errors"
	"os"
	"path/filepath"
	"sort"
	"testing"

	"github.com/openSUSE/libeconf/go/econf"
)

func writeFixture(t *testing.T, dir, name, content string) string {
	t.Helper()
	path := filepath.Join(dir, name)
	if err := os.WriteFile(path, []byte(content), 0o644); err != nil {
		t.Fatalf("writing fixture %s: %v", path, err)
	}
	return path
}

func TestReadFileAndGetters(t *testing.T) {
	dir := t.TempDir()
	path := writeFixture(t, dir, "test.conf", `[main]
name = libeconf
count = 42
ratio = 3.5
enabled = true
quoted = "hello world"
`)

	f, err := econf.ReadFile(path, "=", "#")
	if err != nil {
		t.Fatalf("ReadFile: %v", err)
	}
	defer f.Close()

	if got, err := f.String("main", "name"); err != nil || got != "libeconf" {
		t.Fatalf("String(name) = %q, %v", got, err)
	}
	if got, err := f.Int("main", "count"); err != nil || got != 42 {
		t.Fatalf("Int(count) = %d, %v", got, err)
	}
	if got, err := f.Double("main", "ratio"); err != nil || got != 3.5 {
		t.Fatalf("Double(ratio) = %v, %v", got, err)
	}
	if got, err := f.Bool("main", "enabled"); err != nil || got != true {
		t.Fatalf("Bool(enabled) = %v, %v", got, err)
	}
	if got, err := f.String("main", "quoted"); err != nil || got != "hello world" {
		t.Fatalf("String(quoted) = %q, %v", got, err)
	}
	if !f.HasQuotes("main", "quoted") {
		t.Fatalf("HasQuotes(quoted) = false, want true")
	}

	if _, err := f.String("main", "missing"); !errors.Is(err, econf.ErrNoKey) {
		t.Fatalf("String(missing) error = %v, want ErrNoKey", err)
	}
	if got, err := f.StringDef("main", "missing", "fallback"); err == nil || got != "fallback" {
		t.Fatalf("StringDef(missing) = %q, %v, want fallback with non-nil error", got, err)
	}

	groups, err := f.Groups()
	if err != nil {
		t.Fatalf("Groups: %v", err)
	}
	if len(groups) != 1 || groups[0] != "main" {
		t.Fatalf("Groups() = %v, want [main]", groups)
	}

	keys, err := f.Keys("main")
	if err != nil {
		t.Fatalf("Keys: %v", err)
	}
	sort.Strings(keys)
	want := []string{"count", "enabled", "name", "quoted", "ratio"}
	if len(keys) != len(want) {
		t.Fatalf("Keys(main) = %v, want %v", keys, want)
	}
	for i := range want {
		if keys[i] != want[i] {
			t.Fatalf("Keys(main) = %v, want %v", keys, want)
		}
	}
}

func TestSettersAndWriteRoundTrip(t *testing.T) {
	kf, err := econf.NewIniFile()
	if err != nil {
		t.Fatalf("NewIniFile: %v", err)
	}
	defer kf.Close()

	if err := kf.SetString("main", "name", "libeconf"); err != nil {
		t.Fatalf("SetString: %v", err)
	}
	if err := kf.SetInt("main", "count", 7); err != nil {
		t.Fatalf("SetInt: %v", err)
	}
	if err := kf.SetBool("main", "enabled", true); err != nil {
		t.Fatalf("SetBool: %v", err)
	}
	if err := kf.SetQuotes("main", "name", true); err != nil {
		t.Fatalf("SetQuotes: %v", err)
	}

	dir := t.TempDir()
	if err := kf.WriteFile(dir, "out.conf"); err != nil {
		t.Fatalf("WriteFile: %v", err)
	}

	roundTripped, err := econf.ReadFile(filepath.Join(dir, "out.conf"), "=", "#")
	if err != nil {
		t.Fatalf("ReadFile(roundtrip): %v", err)
	}
	defer roundTripped.Close()

	if got, err := roundTripped.String("main", "name"); err != nil || got != "libeconf" {
		t.Fatalf("roundtrip String(name) = %q, %v", got, err)
	}
	if got, err := roundTripped.Int("main", "count"); err != nil || got != 7 {
		t.Fatalf("roundtrip Int(count) = %d, %v", got, err)
	}
	if got, err := roundTripped.Bool("main", "enabled"); err != nil || got != true {
		t.Fatalf("roundtrip Bool(enabled) = %v, %v", got, err)
	}
}

func TestMergeFiles(t *testing.T) {
	dir := t.TempDir()
	usrPath := writeFixture(t, dir, "usr.conf", "[main]\nname = vendor\nextra = kept\n")
	etcPath := writeFixture(t, dir, "etc.conf", "[main]\nname = admin\n")

	usr, err := econf.ReadFile(usrPath, "=", "#")
	if err != nil {
		t.Fatalf("ReadFile(usr): %v", err)
	}
	defer usr.Close()
	etc, err := econf.ReadFile(etcPath, "=", "#")
	if err != nil {
		t.Fatalf("ReadFile(etc): %v", err)
	}
	defer etc.Close()

	merged, err := econf.MergeFiles(usr, etc)
	if err != nil {
		t.Fatalf("MergeFiles: %v", err)
	}
	defer merged.Close()

	if got, err := merged.String("main", "name"); err != nil || got != "admin" {
		t.Fatalf("merged String(name) = %q, %v, want admin (etc wins)", got, err)
	}
	if got, err := merged.String("main", "extra"); err != nil || got != "kept" {
		t.Fatalf("merged String(extra) = %q, %v, want kept", got, err)
	}
}

func TestReadFileWithCallback(t *testing.T) {
	dir := t.TempDir()
	path := writeFixture(t, dir, "test.conf", "[main]\nname = libeconf\n")

	if _, err := econf.ReadFileWithCallback(path, "=", "#", func(filename string) bool {
		return false
	}); !errors.Is(err, econf.ErrParsingCallbackFailed) {
		t.Fatalf("ReadFileWithCallback(reject) error = %v, want ErrParsingCallbackFailed", err)
	}

	var seen string
	f, err := econf.ReadFileWithCallback(path, "=", "#", func(filename string) bool {
		seen = filename
		return true
	})
	if err != nil {
		t.Fatalf("ReadFileWithCallback(accept): %v", err)
	}
	defer f.Close()

	if seen != path {
		t.Fatalf("callback saw filename %q, want %q", seen, path)
	}
	if got, err := f.String("main", "name"); err != nil || got != "libeconf" {
		t.Fatalf("String(name) = %q, %v", got, err)
	}
}

func TestCloseIsIdempotent(t *testing.T) {
	f, err := econf.NewIniFile()
	if err != nil {
		t.Fatalf("NewIniFile: %v", err)
	}
	if err := f.Close(); err != nil {
		t.Fatalf("Close: %v", err)
	}
	if err := f.Close(); err != nil {
		t.Fatalf("second Close: %v", err)
	}
}
