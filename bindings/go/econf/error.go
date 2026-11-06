package econf

/*
#include <libeconf.h>
*/
import "C"

// Error wraps a non-success econf_err return code.
type Error struct {
	code C.econf_err
}

// Error implements the error interface, returning the human-readable string
// provided by econf_errString.
func (e *Error) Error() string {
	return C.GoString(C.econf_errString(e.code))
}

// Is reports whether target is an *Error wrapping the same econf_err code,
// so callers can use errors.Is(err, econf.ErrNoKey) and similar.
func (e *Error) Is(target error) bool {
	other, ok := target.(*Error)
	return ok && other.code == e.code
}

// Sentinel errors, one per non-success econf_err code, for use with
// errors.Is. See enum econf_err in include/libeconf.h for the authoritative
// description of each code.
var (
	ErrError                 = &Error{C.ECONF_ERROR}
	ErrNoMem                 = &Error{C.ECONF_NOMEM}
	ErrNoFile                = &Error{C.ECONF_NOFILE}
	ErrNoGroup               = &Error{C.ECONF_NOGROUP}
	ErrNoKey                 = &Error{C.ECONF_NOKEY}
	ErrEmptyKey              = &Error{C.ECONF_EMPTYKEY}
	ErrWriteError            = &Error{C.ECONF_WRITEERROR}
	ErrParseError            = &Error{C.ECONF_PARSE_ERROR}
	ErrMissingBracket        = &Error{C.ECONF_MISSING_BRACKET}
	ErrMissingDelimiter      = &Error{C.ECONF_MISSING_DELIMITER}
	ErrEmptySectionName      = &Error{C.ECONF_EMPTY_SECTION_NAME}
	ErrTextAfterSection      = &Error{C.ECONF_TEXT_AFTER_SECTION}
	ErrFileListIsNull        = &Error{C.ECONF_FILE_LIST_IS_NULL}
	ErrWrongBooleanValue     = &Error{C.ECONF_WRONG_BOOLEAN_VALUE}
	ErrKeyHasNullValue       = &Error{C.ECONF_KEY_HAS_NULL_VALUE}
	ErrWrongOwner            = &Error{C.ECONF_WRONG_OWNER}
	ErrWrongGroup            = &Error{C.ECONF_WRONG_GROUP}
	ErrWrongFilePermission   = &Error{C.ECONF_WRONG_FILE_PERMISSION}
	ErrWrongDirPermission    = &Error{C.ECONF_WRONG_DIR_PERMISSION}
	ErrFileIsSymLink         = &Error{C.ECONF_ERROR_FILE_IS_SYM_LINK}
	ErrParsingCallbackFailed = &Error{C.ECONF_PARSING_CALLBACK_FAILED}
	ErrArgumentIsNullValue   = &Error{C.ECONF_ARGUMENT_IS_NULL_VALUE}
	ErrOptionNotFound        = &Error{C.ECONF_OPTION_NOT_FOUND}
	ErrValueConversionError  = &Error{C.ECONF_VALUE_CONVERSION_ERROR}
)

// newError converts a C.econf_err into a Go error, returning nil for
// ECONF_SUCCESS.
func newError(code C.econf_err) error {
	if code == C.ECONF_SUCCESS {
		return nil
	}
	return &Error{code}
}
