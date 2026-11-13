package econf

import (
	"errors"
	"strings"
)

func (f *File) valueAt(num int) string {
	return valueOf(f.entries[num].value)
}

// Int returns the int32 value of group/key. group may be "" if the key is
// not inside a group.
func (f *File) Int(group, key string) (int32, error) {
	num, err := f.findKey(stripBrackets(group), key)
	if err != nil {
		return 0, err
	}
	n, err := parseInt(f.valueAt(num), 32)
	return int32(n), err
}

// IntDef returns the int32 value of group/key, or def if the key is not
// found.
func (f *File) IntDef(group, key string, def int32) (int32, error) {
	v, err := f.Int(group, key)
	if errors.Is(err, ErrNoKey) {
		return def, err
	}
	return v, err
}

// Int64 returns the int64 value of group/key.
func (f *File) Int64(group, key string) (int64, error) {
	num, err := f.findKey(stripBrackets(group), key)
	if err != nil {
		return 0, err
	}
	n, err := parseInt(f.valueAt(num), 64)
	return n, err
}

// Int64Def returns the int64 value of group/key, or def if the key is not
// found.
func (f *File) Int64Def(group, key string, def int64) (int64, error) {
	v, err := f.Int64(group, key)
	if errors.Is(err, ErrNoKey) {
		return def, err
	}
	return v, err
}

// UInt returns the uint32 value of group/key.
func (f *File) UInt(group, key string) (uint32, error) {
	num, err := f.findKey(stripBrackets(group), key)
	if err != nil {
		return 0, err
	}
	n, err := parseUint(f.valueAt(num), 32)
	return uint32(n), err
}

// UIntDef returns the uint32 value of group/key, or def if the key is not
// found.
func (f *File) UIntDef(group, key string, def uint32) (uint32, error) {
	v, err := f.UInt(group, key)
	if errors.Is(err, ErrNoKey) {
		return def, err
	}
	return v, err
}

// UInt64 returns the uint64 value of group/key.
func (f *File) UInt64(group, key string) (uint64, error) {
	num, err := f.findKey(stripBrackets(group), key)
	if err != nil {
		return 0, err
	}
	n, err := parseUint(f.valueAt(num), 64)
	return n, err
}

// UInt64Def returns the uint64 value of group/key, or def if the key is not
// found.
func (f *File) UInt64Def(group, key string, def uint64) (uint64, error) {
	v, err := f.UInt64(group, key)
	if errors.Is(err, ErrNoKey) {
		return def, err
	}
	return v, err
}

// Float returns the float32 value of group/key.
func (f *File) Float(group, key string) (float32, error) {
	num, err := f.findKey(stripBrackets(group), key)
	if err != nil {
		return 0, err
	}
	v, err := parseFloat(f.valueAt(num), 32)
	return float32(v), err
}

// FloatDef returns the float32 value of group/key, or def if the key is
// not found.
func (f *File) FloatDef(group, key string, def float32) (float32, error) {
	v, err := f.Float(group, key)
	if errors.Is(err, ErrNoKey) {
		return def, err
	}
	return v, err
}

// Double returns the float64 value of group/key.
func (f *File) Double(group, key string) (float64, error) {
	num, err := f.findKey(stripBrackets(group), key)
	if err != nil {
		return 0, err
	}
	v, err := parseFloat(f.valueAt(num), 64)
	return v, err
}

// DoubleDef returns the float64 value of group/key, or def if the key is
// not found.
func (f *File) DoubleDef(group, key string, def float64) (float64, error) {
	v, err := f.Double(group, key)
	if errors.Is(err, ErrNoKey) {
		return def, err
	}
	return v, err
}

// String returns the string value of group/key.
func (f *File) String(group, key string) (string, error) {
	num, err := f.findKey(stripBrackets(group), key)
	if err != nil {
		return "", err
	}
	return f.valueAt(num), nil
}

// StringDef returns the string value of group/key, or def if the key is
// not found.
func (f *File) StringDef(group, key, def string) (string, error) {
	v, err := f.String(group, key)
	if errors.Is(err, ErrNoKey) {
		return def, err
	}
	return v, err
}

// parseBool mirrors keyfile.c's getBoolValueNum.
func parseBool(value string) (bool, error) {
	lower := strings.ToLower(value)
	switch {
	case (value == "1" && len(value) == 1) || lower == "yes" || lower == "true":
		return true, nil
	case (value == "0" && len(value) == 1) || value == "" || lower == "no" || lower == "false":
		return false, nil
	case lower == noGroup:
		return false, ErrKeyHasNullValue
	default:
		return false, ErrParseError
	}
}

// Bool returns the bool value of group/key.
func (f *File) Bool(group, key string) (bool, error) {
	num, err := f.findKey(stripBrackets(group), key)
	if err != nil {
		return false, err
	}
	return parseBool(f.valueAt(num))
}

// BoolDef returns the bool value of group/key, or def if the key is not
// found.
func (f *File) BoolDef(group, key string, def bool) (bool, error) {
	v, err := f.Bool(group, key)
	if errors.Is(err, ErrNoKey) {
		return def, err
	}
	return v, err
}

// HasQuotes reports whether the value of group/key is enclosed by quotes.
func (f *File) HasQuotes(group, key string) bool {
	num, err := f.findKey(stripBrackets(group), key)
	if err != nil {
		return false
	}
	return f.entries[num].quotes
}
