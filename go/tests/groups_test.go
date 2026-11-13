package econf_test

import (
	"errors"
	"path/filepath"
	"testing"

	"github.com/openSUSE/libeconf/go/econf"
)

// checkGroupString mirrors tst-groups1.c's check_String: it sets "KEY" under
// setGroup and verifies it reads back unchanged under getGroup (which may
// use a different, but equivalent, bracketed/unbracketed spelling of the
// same group name).
func checkGroupString(t *testing.T, f *econf.File, value, setGroup, getGroup string) {
	t.Helper()
	if err := f.SetString(setGroup, "KEY", value); err != nil {
		t.Fatalf("SetString(%q, KEY, %q): %v", setGroup, value, err)
	}
	got, err := f.String(getGroup, "KEY")
	if err != nil {
		t.Fatalf("String(%q, KEY) after SetString(%q, ...): %v", getGroup, setGroup, err)
	}
	if got != value {
		t.Fatalf("String(%q, KEY) = %q, want %q (set via group %q)", getGroup, got, value, setGroup)
	}
}

// TestGroupsSetAndGetRoundTrip replicates tst-groups1.c: storing and
// re-reading a string value across various equivalent spellings of a group
// name (plain, bracketed, and the no-group case).
func TestGroupsSetAndGetRoundTrip(t *testing.T) {
	f, err := econf.NewIniFile()
	if err != nil {
		t.Fatalf("NewIniFile: %v", err)
	}
	defer f.Close()

	// Reading a key from a brand new, empty key file must fail with
	// ErrNoKey, not panic or succeed.
	if _, err := f.Int("doesnotexist", "KEY"); !errors.Is(err, econf.ErrNoKey) {
		t.Fatalf("Int(doesnotexist, KEY) on empty file = %v, want ErrNoKey", err)
	}

	checkGroupString(t, f, "", "", "")
	checkGroupString(t, f, "", "group1", "[group1]")
	checkGroupString(t, f, "", "group2", "[group2]")
	checkGroupString(t, f, "TaTa", "[group2]", "group2")
	checkGroupString(t, f, "TeST", "[group3]", "[group3]")
	checkGroupString(t, f, "dummy", "group4", "group4")
}

// TestGroupsSingleGroup replicates tst-groups2.c: a key file with exactly
// one group/key must list exactly that one group.
func TestGroupsSingleGroup(t *testing.T) {
	f, err := econf.NewIniFile()
	if err != nil {
		t.Fatalf("NewIniFile: %v", err)
	}
	defer f.Close()

	const group, key, val = "towel", "42", "The answer, not question"
	if err := f.SetString(group, key, val); err != nil {
		t.Fatalf("SetString: %v", err)
	}

	if got, err := f.String(group, key); err != nil || got != val {
		t.Fatalf("String(%q, %q) = %q, %v, want %q", group, key, got, err, val)
	}

	groups, err := f.Groups()
	if err != nil {
		t.Fatalf("Groups: %v", err)
	}
	if len(groups) != 1 || groups[0] != group {
		t.Fatalf("Groups() = %v, want [%s]", groups, group)
	}
}

// TestGroupsListFromFile replicates tst-groups3.c: reading a file with a
// mix of bracketed group headers (and one malformed, bracket-less line that
// must not be mistaken for a group of its own) lists exactly the bracketed
// groups, each with a readable "key" value.
func TestGroupsListFromFile(t *testing.T) {
	path := filepath.Join("..", "..", "tests", "tst-groups3-data", "groups.conf")
	f, err := econf.ReadFile(path, "=", "#")
	if err != nil {
		t.Fatalf("ReadFile: %v", err)
	}
	defer f.Close()

	groups, err := f.Groups()
	if err != nil {
		t.Fatalf("Groups: %v", err)
	}
	wantGroups := []string{"global", "section2", "blablabla", "section5", "section6"}
	if len(groups) != len(wantGroups) {
		t.Fatalf("Groups() = %v, want %v", groups, wantGroups)
	}
	for i := range wantGroups {
		if groups[i] != wantGroups[i] {
			t.Fatalf("Groups() = %v, want %v", groups, wantGroups)
		}
	}

	// Each group's "key" keeps the value from its first assignment; the
	// bracket-less "section3" line between section2's two "key=" lines is
	// not itself a group header and does not clobber the earlier value.
	wantValues := map[string]string{
		"global":    "global",
		"section2":  "section2",
		"blablabla": "blablabla",
		"section5":  "section5",
		"section6":  "section6",
	}
	for _, g := range groups {
		got, err := f.String(g, "key")
		if err != nil {
			t.Errorf("String(%q, key): %v", g, err)
			continue
		}
		if got != wantValues[g] {
			t.Errorf("String(%q, key) = %q, want %q", g, got, wantValues[g])
		}
	}
}

// TestGroupsSingleEmptyGroup replicates tst-groups5.c: a file containing
// only an empty "[global]" group header (no keys) still reports that one
// group.
func TestGroupsSingleEmptyGroup(t *testing.T) {
	path := filepath.Join("..", "..", "tests", "tst-groups5-data", "groups.conf")
	f, err := econf.ReadFile(path, "=", "#")
	if err != nil {
		t.Fatalf("ReadFile: %v", err)
	}
	defer f.Close()

	groups, err := f.Groups()
	if err != nil {
		t.Fatalf("Groups: %v", err)
	}
	if len(groups) != 1 || groups[0] != "global" {
		t.Fatalf("Groups() = %v, want [global]", groups)
	}
}

// TestGroupsListKeysPerGroup replicates tst-groups6.c: listing groups and,
// for each, its keys and their values.
func TestGroupsListKeysPerGroup(t *testing.T) {
	path := filepath.Join("..", "..", "tests", "tst-groups6-data", "groups.conf")
	f, err := econf.ReadFile(path, "=", "#")
	if err != nil {
		t.Fatalf("ReadFile: %v", err)
	}
	defer f.Close()

	groups, err := f.Groups()
	if err != nil {
		t.Fatalf("Groups: %v", err)
	}
	wantGroups := []string{"section1", "section2", "section3"}
	if len(groups) != len(wantGroups) {
		t.Fatalf("Groups() = %v, want %v", groups, wantGroups)
	}
	for i := range wantGroups {
		if groups[i] != wantGroups[i] {
			t.Fatalf("Groups() = %v, want %v", groups, wantGroups)
		}
	}

	keys, err := f.Keys("section1")
	if err != nil {
		t.Fatalf("Keys(section1): %v", err)
	}
	if len(keys) != 3 {
		t.Fatalf("Keys(section1) = %v, want 3 keys", keys)
	}

	checks := []struct{ group, key, want string }{
		{"section1", "key", "yes"},
		{"section1", "key1", "no"},
		{"section1", "key2", "yes"},
		{"section2", "key", "yes"},
		{"section3", "key", "yes"},
	}
	for _, c := range checks {
		got, err := f.String(c.group, c.key)
		if err != nil {
			t.Errorf("String(%q, %q): %v", c.group, c.key, err)
			continue
		}
		if got != c.want {
			t.Errorf("String(%q, %q) = %q, want %q", c.group, c.key, got, c.want)
		}
	}
}
