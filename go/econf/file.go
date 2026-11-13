package econf

import "strings"

// Close is a no-op; there is no C memory to release in this pure-Go
// implementation. It always returns nil and is safe to call multiple
// times and on a nil *File.
func (f *File) Close() error {
	return nil
}

// NewKeyFile creates a new, empty econf_file with the given delimiter and
// comment characters.
//
// Unlike NewKeyFileWithOptions, the C implementation this mirrors
// (econf_newKeyFile) eagerly pre-allocates a handful of placeholder
// entries as a capacity optimization, which as a side effect seeds the
// internal group-name list with the "no group" sentinel. That makes
// Groups() succeed with an empty result for a freshly created (but still
// entirely empty) *File from NewKeyFile/NewIniFile, whereas it returns
// ErrNoGroup for one from NewKeyFileWithOptions or a freshly parsed empty
// file.
func NewKeyFile(delimiter, comment byte) (*File, error) {
	f := &File{delimiter: delimiter, comment: comment}
	f.internGroup(noGroup)
	return f, nil
}

// NewKeyFileWithOptions creates a new, empty econf_file configured by a
// ";"-separated "key=value" options string (e.g. "JOIN_SAME_ENTRIES=1").
// See econf_newKeyFile_with_options in include/libeconf.h for the full list
// of supported options.
func NewKeyFileWithOptions(options string) (*File, error) {
	f := &File{}
	if options == "" {
		return f, nil
	}
	for _, opt := range strings.Split(options, ";") {
		switch {
		case opt == "JOIN_SAME_ENTRIES=1":
			f.joinSameEntries = true
		case opt == "PYTHON_STYLE=1":
			f.pythonStyle = true
		case opt == "DEFAULT_QUOTES=1":
			f.defaultQuotes = true
		case strings.HasPrefix(opt, "PARSING_DIRS="):
			f.parseDirs = strings.Split(strings.TrimPrefix(opt, "PARSING_DIRS="), ":")
		case strings.HasPrefix(opt, "CONFIG_DIRS="):
			f.confDirs = strings.Split(strings.TrimPrefix(opt, "CONFIG_DIRS="), ":")
		case strings.HasPrefix(opt, "ROOT_PREFIX="):
			f.rootPrefix = strings.TrimPrefix(opt, "ROOT_PREFIX=")
		default:
			return nil, ErrOptionNotFound
		}
	}
	return f, nil
}

// NewIniFile creates a new econf_file in INI format ("=" delimiter, "#"
// comments).
func NewIniFile() (*File, error) {
	return NewKeyFile('=', '#')
}

// Path returns the absolute path of the parsed file, or an empty string for
// already-merged data.
func (f *File) Path() string {
	return f.path
}

// Groups returns all group names found in f, in the order they were first
// interned (see f.groupNames). If f has no entries and no group headers
// were ever seen, it returns ErrNoGroup; a file that only ever saw
// ungrouped entries, or group headers with zero entries under them,
// returns a possibly-empty slice with a nil error.
func (f *File) Groups() ([]string, error) {
	if len(f.groupNames) == 0 {
		return nil, ErrNoGroup
	}
	var groups []string
	for _, g := range f.groupNames {
		if g != noGroup {
			groups = append(groups, g)
		}
	}
	return groups, nil
}

// Keys returns all keys in the given group. If group is "", keys that have
// no group are returned.
func (f *File) Keys(group string) ([]string, error) {
	grp := groupOrNone(group)
	var keys []string
	for _, e := range f.entries {
		if e.group == grp {
			keys = append(keys, e.key)
		}
	}
	if len(keys) == 0 {
		return nil, ErrNoKey
	}
	return keys, nil
}

// CommentTag returns the comment character tag of f.
func (f *File) CommentTag() byte {
	return f.comment
}

// SetCommentTag sets the comment character tag of f.
func (f *File) SetCommentTag(comment byte) {
	f.comment = comment
}

// DelimiterTag returns the key/value delimiter character of f.
func (f *File) DelimiterTag() byte {
	return f.delimiter
}

// SetDelimiterTag sets the key/value delimiter character of f.
func (f *File) SetDelimiterTag(delimiter byte) {
	f.delimiter = delimiter
}
