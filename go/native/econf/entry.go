package econf

import "errors"

// noGroup is the sentinel group name used internally for entries that have
// no group, mirroring KEY_FILE_NULL_VALUE ("_none_") in lib/defines.h. The
// public API uses "" for the same concept; the two are translated at the
// package boundary.
const noGroup = "_none_"

// entry mirrors struct file_entry in lib/keyfile.h. value, commentBeforeKey
// and commentAfterValue are *string rather than string so a stored-but-
// empty value ("") can be distinguished from "no value at all" (nil),
// matching the C struct's NULL vs. "" distinction.
type entry struct {
	group             string
	key               string
	value             *string
	commentBeforeKey  *string
	commentAfterValue *string
	lineNumber        uint64
	quotes            bool
}

// File wraps an in-memory representation of a parsed and/or merged
// configuration, mirroring the C econf_file struct in lib/keyfile.h.
type File struct {
	entries []entry

	delimiter byte
	comment   byte
	path      string

	joinSameEntries bool
	pythonStyle     bool
	defaultQuotes   bool

	parseDirs  []string
	confDirs   []string
	rootPrefix string

	// groupNames is the set of group names ever interned into f, in order
	// of first appearance - including noGroup once any ungrouped entry (or
	// NewKeyFile's seeding; see its doc comment) has been recorded, and
	// including groups whose header was parsed even if no key/value ever
	// followed it. This mirrors the C library's groups[] interning array,
	// which is intentionally never pruned. Groups() is a thin filter over
	// this list rather than a derivation from entries, since a group with
	// zero entries is otherwise invisible by scanning entries alone.
	groupNames []string
}

func groupOrNone(group string) string {
	if group == "" {
		return noGroup
	}
	return group
}

// internGroup records name in f.groupNames if not already present,
// mirroring helpers.c's setGroupList.
func (f *File) internGroup(name string) {
	for _, g := range f.groupNames {
		if g == name {
			return
		}
	}
	f.groupNames = append(f.groupNames, name)
}

// stripBrackets mirrors helpers.c's stripbrackets: it removes a leading
// '[' and trailing ']' from group, but only if both are present.
func stripBrackets(group string) string {
	if len(group) >= 2 && group[0] == '[' && group[len(group)-1] == ']' {
		return group[1 : len(group)-1]
	}
	return group
}

// addBrackets mirrors helpers.c's addbrackets, used when writing group
// headers.
func addBrackets(group string) string {
	if len(group) >= 2 && group[0] == '[' && group[len(group)-1] == ']' {
		return group
	}
	return "[" + group + "]"
}

// findKey mirrors helpers.c's find_key.
func (f *File) findKey(group, key string) (int, error) {
	grp := groupOrNone(group)
	if key == "" {
		return -1, ErrError
	}
	for i := range f.entries {
		if f.entries[i].group == grp && f.entries[i].key == key {
			return i, nil
		}
	}
	return -1, ErrNoKey
}

// newKey appends a brand-new entry for group/key and returns its index,
// mirroring helpers.c's new_key.
func (f *File) newKey(group, key string) int {
	grp := groupOrNone(group)
	f.internGroup(grp)
	f.entries = append(f.entries, entry{
		group:  grp,
		key:    key,
		quotes: f.defaultQuotes,
	})
	return len(f.entries) - 1
}

// setKeyValue finds (or creates) the entry for group/key and applies set
// to it, mirroring helpers.c's setKeyValue.
func (f *File) setKeyValue(group, key string, set func(e *entry)) error {
	num, err := f.findKey(group, key)
	if err != nil {
		if !errors.Is(err, ErrNoKey) {
			return err
		}
		num = f.newKey(group, key)
	}
	set(&f.entries[num])
	return nil
}

func isSpaceC(b byte) bool {
	switch b {
	case ' ', '\t', '\n', '\v', '\f', '\r':
		return true
	}
	return false
}

func valueOf(v *string) string {
	if v == nil {
		return ""
	}
	return *v
}
