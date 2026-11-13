package econf_test

import (
	"errors"
	"path/filepath"
	"testing"

	"github.com/openSUSE/libeconf/go/econf"
)

// TestParseErrorCodes replicates the error-code part of tst-parse-error.c:
// each malformed input is rejected with its specific error, not a generic
// one.
//
// Not ported: the econf_errLocation(filename, line_nr) checks. This
// package does not track or expose the file/line location of the last
// parse error at all (not even internally - the parser's error returns
// carry no location state), so there is nothing to port those assertions
// against.
func TestParseErrorCodes(t *testing.T) {
	t.Run("missing bracket", func(t *testing.T) {
		// econf_readDirs isn't exposed (see getconfdirs_test.go); only the
		// error code is under test here, so ReadDirsHistory is sufficient
		// (no need for the mergeHistory step).
		base := filepath.Join("..", "..", "tests", "tst-parse-error")
		_, err := econf.ReadDirsHistory(filepath.Join(base, "usr", "etc"), filepath.Join(base, "etc"), "missing_bracket", "conf", "=", "#")
		if !errors.Is(err, econf.ErrMissingBracket) {
			t.Errorf("error = %v, want ErrMissingBracket", err)
		}
	})

	t.Run("missing delimiter", func(t *testing.T) {
		path := filepath.Join("..", "..", "tests", "tst-parse-error", "missing_delim.conf")
		_, err := econf.ReadFile(path, "=", "#")
		if !errors.Is(err, econf.ErrMissingDelimiter) {
			t.Errorf("error = %v, want ErrMissingDelimiter", err)
		}
	})

	t.Run("empty section name", func(t *testing.T) {
		path := filepath.Join("..", "..", "tests", "tst-parse-error", "empty_section.conf")
		_, err := econf.ReadFile(path, "=", "#")
		if !errors.Is(err, econf.ErrEmptySectionName) {
			t.Errorf("error = %v, want ErrEmptySectionName", err)
		}
	})

	t.Run("text after section", func(t *testing.T) {
		path := filepath.Join("..", "..", "tests", "tst-parse-error", "text_after_section.conf")
		_, err := econf.ReadFile(path, "=", "#")
		if !errors.Is(err, econf.ErrTextAfterSection) {
			t.Errorf("error = %v, want ErrTextAfterSection", err)
		}
	})
}
