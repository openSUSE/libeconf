package econf_test

import (
	"errors"
	"math"
	"testing"

	"github.com/openSUSE/libeconf/go/econf"
)

// TestSetGetAllValueTypes replicates the round-trip part of
// tst-setgetvalues1.c: every Set*/Get* pair stores and reads back the same
// value, including type-extreme values.
func TestSetGetAllValueTypes(t *testing.T) {
	f, err := econf.NewIniFile()
	if err != nil {
		t.Fatalf("NewIniFile: %v", err)
	}
	defer f.Close()

	if _, err := f.Int("", "KEY"); !errors.Is(err, econf.ErrNoKey) {
		t.Fatalf("Int(KEY) on empty file = %v, want ErrNoKey", err)
	}

	if err := f.SetInt("", "KEY", math.MaxInt32); err != nil {
		t.Fatalf("SetInt(MaxInt32): %v", err)
	}
	if got, err := f.Int("", "KEY"); err != nil || got != math.MaxInt32 {
		t.Errorf("Int() = %d, %v, want %d", got, err, int32(math.MaxInt32))
	}
	if err := f.SetInt("", "KEY", math.MinInt32); err != nil {
		t.Fatalf("SetInt(MinInt32): %v", err)
	}
	if got, err := f.Int("", "KEY"); err != nil || got != math.MinInt32 {
		t.Errorf("Int() = %d, %v, want %d", got, err, int32(math.MinInt32))
	}

	if err := f.SetUInt("", "KEY", math.MaxUint32); err != nil {
		t.Fatalf("SetUInt(MaxUint32): %v", err)
	}
	if got, err := f.UInt("", "KEY"); err != nil || got != math.MaxUint32 {
		t.Errorf("UInt() = %d, %v, want %d", got, err, uint32(math.MaxUint32))
	}
	if err := f.SetUInt("", "KEY", 0); err != nil {
		t.Fatalf("SetUInt(0): %v", err)
	}
	if got, err := f.UInt("", "KEY"); err != nil || got != 0 {
		t.Errorf("UInt() = %d, %v, want 0", got, err)
	}

	if err := f.SetInt64("", "KEY", math.MaxInt64); err != nil {
		t.Fatalf("SetInt64(MaxInt64): %v", err)
	}
	if got, err := f.Int64("", "KEY"); err != nil || got != math.MaxInt64 {
		t.Errorf("Int64() = %d, %v, want %d", got, err, int64(math.MaxInt64))
	}
	if err := f.SetInt64("", "KEY", math.MinInt64); err != nil {
		t.Fatalf("SetInt64(MinInt64): %v", err)
	}
	if got, err := f.Int64("", "KEY"); err != nil || got != math.MinInt64 {
		t.Errorf("Int64() = %d, %v, want %d", got, err, int64(math.MinInt64))
	}

	if err := f.SetUInt64("", "KEY", math.MaxUint64); err != nil {
		t.Fatalf("SetUInt64(MaxUint64): %v", err)
	}
	if got, err := f.UInt64("", "KEY"); err != nil || got != math.MaxUint64 {
		t.Errorf("UInt64() = %d, %v, want %d", got, err, uint64(math.MaxUint64))
	}
	if err := f.SetUInt64("", "KEY", 0); err != nil {
		t.Fatalf("SetUInt64(0): %v", err)
	}
	if got, err := f.UInt64("", "KEY"); err != nil || got != 0 {
		t.Errorf("UInt64() = %d, %v, want 0", got, err)
	}

	// The smallest positive *normalized* values (C's FLT_MIN/DBL_MIN), not
	// Go's math.SmallestNonzeroFloat32/64 (the smallest *denormalized*
	// values): the cgo binding's underlying C library fails to round-trip
	// a denormalized double, so this sticks to the C test's original
	// FLT_MIN/DBL_MIN to stay within what both implementations support.
	const fltMin = 1.17549435e-38
	const dblMin = 2.2250738585072014e-308

	if err := f.SetFloat("", "KEY", math.MaxFloat32); err != nil {
		t.Fatalf("SetFloat(MaxFloat32): %v", err)
	}
	if got, err := f.Float("", "KEY"); err != nil || got != math.MaxFloat32 {
		t.Errorf("Float() = %v, %v, want %v", got, err, float32(math.MaxFloat32))
	}
	if err := f.SetFloat("", "KEY", fltMin); err != nil {
		t.Fatalf("SetFloat(FLT_MIN): %v", err)
	}
	if got, err := f.Float("", "KEY"); err != nil || got != fltMin {
		t.Errorf("Float() = %v, %v, want %v", got, err, float32(fltMin))
	}

	if err := f.SetDouble("", "KEY", math.MaxFloat64); err != nil {
		t.Fatalf("SetDouble(MaxFloat64): %v", err)
	}
	if got, err := f.Double("", "KEY"); err != nil || got != math.MaxFloat64 {
		t.Errorf("Double() = %v, %v, want %v", got, err, math.MaxFloat64)
	}
	if err := f.SetDouble("", "KEY", dblMin); err != nil {
		t.Fatalf("SetDouble(DBL_MIN): %v", err)
	}
	if got, err := f.Double("", "KEY"); err != nil || got != dblMin {
		t.Errorf("Double() = %v, %v, want %v", got, err, dblMin)
	}

	stringValues := []string{"", " ", "This should become a long and complicated string, but up to now it is not = ! $ % € ß"}
	for _, sv := range stringValues {
		if err := f.SetString("", "KEY", sv); err != nil {
			t.Fatalf("SetString(%q): %v", sv, err)
		}
		if got, err := f.String("", "KEY"); err != nil || got != sv {
			t.Errorf("String() = %q, %v, want %q", got, err, sv)
		}
	}

	if err := f.SetBool("", "KEY", true); err != nil {
		t.Fatalf("SetBool(true): %v", err)
	}
	if got, err := f.Bool("", "KEY"); err != nil || got != true {
		t.Errorf("Bool() = %v, %v, want true", got, err)
	}
	if err := f.SetBool("", "KEY", false); err != nil {
		t.Fatalf("SetBool(false): %v", err)
	}
	if got, err := f.Bool("", "KEY"); err != nil || got != false {
		t.Errorf("Bool() = %v, %v, want false", got, err)
	}
}

// TestBoolStringParsing replicates the boolean-string-parsing part of
// tst-setgetvalues1.c: econf_setBoolValue there accepts an arbitrary
// string ("True"/"yes"/"1"/...) and stores it verbatim, with
// econf_getBoolValue doing the actual parsing. Go's SetBool only accepts a
// real bool (always valid), so there is no Go equivalent of "set an
// arbitrary string via the bool setter"; this exercises the same parsing
// logic by storing the string via SetString and reading it with Bool
// instead, which uses the identical value-parsing code path.
func TestBoolStringParsing(t *testing.T) {
	f, err := econf.NewIniFile()
	if err != nil {
		t.Fatalf("NewIniFile: %v", err)
	}
	defer f.Close()

	tests := []struct {
		raw  string
		want bool
	}{
		{"True", true}, {"true", true}, {"Yes", true}, {"yes", true}, {"1", true},
		{"False", false}, {"false", false}, {"No", false}, {"no", false}, {"0", false},
	}
	for _, tc := range tests {
		if err := f.SetString("", "KEY", tc.raw); err != nil {
			t.Fatalf("SetString(%q): %v", tc.raw, err)
		}
		got, err := f.Bool("", "KEY")
		if err != nil {
			t.Errorf("Bool() after SetString(%q): %v", tc.raw, err)
			continue
		}
		if got != tc.want {
			t.Errorf("Bool() after SetString(%q) = %v, want %v", tc.raw, got, tc.want)
		}
	}
}

// TestSetValueErrorHandling replicates the "Error Handling" part of
// tst-setgetvalues1.c: an empty key is rejected with ErrEmptyKey.
//
// Not ported: econf_setIntValue(NULL, ...) (NULL key_file; calling a
// method on a nil *File panics in Go rather than returning an error, as
// elsewhere), and econf_setBoolValue(..., "foo") / "" / NULL expecting
// ECONF_WRONG_BOOLEAN_VALUE / ECONF_KEY_HAS_NULL_VALUE. The latter two
// require the string-accepting econf_setBoolValue, which Go's SetBool
// (real bool parameter) has no equivalent of; storing the same strings via
// SetString and reading with Bool (as in TestBoolStringParsing) instead
// yields ErrParseError for "foo" and no error (false) for "", since that
// code path does generic value parsing rather than setBoolValue's
// dedicated input validation.
func TestSetValueErrorHandling(t *testing.T) {
	f, err := econf.NewIniFile()
	if err != nil {
		t.Fatalf("NewIniFile: %v", err)
	}
	defer f.Close()

	if err := f.SetInt("", "", 8); !errors.Is(err, econf.ErrEmptyKey) {
		t.Errorf("SetInt(\"\", 8) error = %v, want ErrEmptyKey", err)
	}
}
