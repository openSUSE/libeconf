package econf_test

import (
	"errors"
	"testing"

	"github.com/openSUSE/libeconf/go/econf"
)

// TestReadConfigNonExistent replicates tst-readConfig1.c: calling
// ReadConfig for a project/config pair that doesn't exist anywhere fails
// with ErrNoFile (the original C test's actual concern - that this
// shouldn't leak memory - isn't something a Go test can observe directly,
// but the error-handling path it exercises is the same).
func TestReadConfigNonExistent(t *testing.T) {
	_, err := econf.ReadConfig("foo", "/usr/etc", "bar", "conf", "=", "#")
	if !errors.Is(err, econf.ErrNoFile) {
		t.Errorf("ReadConfig(nonexistent) error = %v, want ErrNoFile", err)
	}
}
