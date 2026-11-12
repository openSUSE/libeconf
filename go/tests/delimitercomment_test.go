package econf_test

import (
	"testing"

	"github.com/openSUSE/libeconf/go/econf"
)

// TestDelimiterAndCommentTag replicates tst-delimiter-comment.c:
// NewIniFile defaults to '=' and '#', and both are independently settable.
func TestDelimiterAndCommentTag(t *testing.T) {
	f, err := econf.NewIniFile()
	if err != nil {
		t.Fatalf("NewIniFile: %v", err)
	}
	defer f.Close()

	if got := f.CommentTag(); got != '#' {
		t.Errorf("CommentTag() = %q, want '#'", got)
	}
	if got := f.DelimiterTag(); got != '=' {
		t.Errorf("DelimiterTag() = %q, want '='", got)
	}

	f.SetCommentTag('-')
	f.SetDelimiterTag(' ')

	if got := f.CommentTag(); got != '-' {
		t.Errorf("CommentTag() = %q, want '-'", got)
	}
	if got := f.DelimiterTag(); got != ' ' {
		t.Errorf("DelimiterTag() = %q, want ' '", got)
	}
}
