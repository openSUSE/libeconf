package econf

// errCode mirrors the C library's econf_err enum (see enum econf_err in
// include/libeconf.h) value for value, so the numeric codes stay stable
// even though nothing outside this package observes the raw integer.
type errCode int

const (
	errSuccess errCode = iota
	errError
	errNoMem
	errNoFile
	errNoGroup
	errNoKey
	errEmptyKey
	errWriteError
	errParseError
	errMissingBracket
	errMissingDelimiter
	errEmptySectionName
	errTextAfterSection
	errFileListIsNull
	errWrongBooleanValue
	errKeyHasNullValue
	errWrongOwner
	errWrongGroup
	errWrongFilePermission
	errWrongDirPermission
	errFileIsSymLink
	errParsingCallbackFailed
	errArgumentIsNullValue
	errOptionNotFound
	errValueConversionError
)

// messages holds the human-readable string for each errCode, copied
// verbatim from the messages[] table in lib/econf_error.c.
var messages = [...]string{
	errSuccess:               "Success",
	errError:                 "Unknown error",
	errNoMem:                 "Out of memory",
	errNoFile:                "Configuration file not found",
	errNoGroup:               "Group not found",
	errNoKey:                 "Key not found",
	errEmptyKey:              "Key is NULL or has empty value",
	errWriteError:            "Error creating or writing to a file",
	errParseError:            "Parse error",
	errMissingBracket:        "Missing bracket",
	errMissingDelimiter:      "Missing delimiter",
	errEmptySectionName:      "Empty section name",
	errTextAfterSection:      "Text after section",
	errFileListIsNull:        "Conf file list is NULL",
	errWrongBooleanValue:     "Wrong boolean value (1/0 true/false yes/no)",
	errKeyHasNullValue:       "Given key has NULL value",
	errWrongOwner:            "File has wrong owner",
	errWrongGroup:            "File has wrong group",
	errWrongFilePermission:   "File has wrong file permissions",
	errWrongDirPermission:    "File has wrong dir permissions",
	errFileIsSymLink:         "File is a sym link which is not permitted",
	errParsingCallbackFailed: "User defined parsing callback has failed",
	errArgumentIsNullValue:   "Given argument is NULL",
	errOptionNotFound:        "Given option not found",
	errValueConversionError:  "Value cannot be converted",
}

// Error wraps a non-success error code returned by the library.
type Error struct {
	code errCode
}

// Error implements the error interface.
func (e *Error) Error() string {
	if int(e.code) < 0 || int(e.code) >= len(messages) {
		return "Unknown libeconf error"
	}
	return messages[e.code]
}

// Is reports whether target is an *Error wrapping the same code, so
// callers can use errors.Is(err, econf.ErrNoKey) and similar.
func (e *Error) Is(target error) bool {
	other, ok := target.(*Error)
	return ok && other.code == e.code
}

// Sentinel errors, one per non-success error code, for use with
// errors.Is. See enum econf_err in include/libeconf.h for the
// authoritative description of each code.
var (
	ErrError                 = &Error{errError}
	ErrNoMem                 = &Error{errNoMem}
	ErrNoFile                = &Error{errNoFile}
	ErrNoGroup               = &Error{errNoGroup}
	ErrNoKey                 = &Error{errNoKey}
	ErrEmptyKey              = &Error{errEmptyKey}
	ErrWriteError            = &Error{errWriteError}
	ErrParseError            = &Error{errParseError}
	ErrMissingBracket        = &Error{errMissingBracket}
	ErrMissingDelimiter      = &Error{errMissingDelimiter}
	ErrEmptySectionName      = &Error{errEmptySectionName}
	ErrTextAfterSection      = &Error{errTextAfterSection}
	ErrFileListIsNull        = &Error{errFileListIsNull}
	ErrWrongBooleanValue     = &Error{errWrongBooleanValue}
	ErrKeyHasNullValue       = &Error{errKeyHasNullValue}
	ErrWrongOwner            = &Error{errWrongOwner}
	ErrWrongGroup            = &Error{errWrongGroup}
	ErrWrongFilePermission   = &Error{errWrongFilePermission}
	ErrWrongDirPermission    = &Error{errWrongDirPermission}
	ErrFileIsSymLink         = &Error{errFileIsSymLink}
	ErrParsingCallbackFailed = &Error{errParsingCallbackFailed}
	ErrArgumentIsNullValue   = &Error{errArgumentIsNullValue}
	ErrOptionNotFound        = &Error{errOptionNotFound}
	ErrValueConversionError  = &Error{errValueConversionError}
)

// newError converts an errCode into a Go error, returning nil for
// errSuccess.
func newError(code errCode) error {
	if code == errSuccess {
		return nil
	}
	return &Error{code}
}
