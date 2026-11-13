package econf

import "strconv"

// SetInt sets the int32 value of group/key.
func (f *File) SetInt(group, key string, value int32) error {
	if key == "" {
		return ErrEmptyKey
	}
	s := strconv.FormatInt(int64(value), 10)
	return f.setKeyValue(stripBrackets(group), key, func(e *entry) { e.value = &s })
}

// SetInt64 sets the int64 value of group/key.
func (f *File) SetInt64(group, key string, value int64) error {
	if key == "" {
		return ErrEmptyKey
	}
	s := strconv.FormatInt(value, 10)
	return f.setKeyValue(stripBrackets(group), key, func(e *entry) { e.value = &s })
}

// SetUInt sets the uint32 value of group/key.
func (f *File) SetUInt(group, key string, value uint32) error {
	if key == "" {
		return ErrEmptyKey
	}
	s := strconv.FormatUint(uint64(value), 10)
	return f.setKeyValue(stripBrackets(group), key, func(e *entry) { e.value = &s })
}

// SetUInt64 sets the uint64 value of group/key.
func (f *File) SetUInt64(group, key string, value uint64) error {
	if key == "" {
		return ErrEmptyKey
	}
	s := strconv.FormatUint(value, 10)
	return f.setKeyValue(stripBrackets(group), key, func(e *entry) { e.value = &s })
}

// SetFloat sets the float32 value of group/key.
func (f *File) SetFloat(group, key string, value float32) error {
	if key == "" {
		return ErrEmptyKey
	}
	s := strconv.FormatFloat(float64(value), 'g', -1, 32)
	return f.setKeyValue(stripBrackets(group), key, func(e *entry) { e.value = &s })
}

// SetDouble sets the float64 value of group/key.
func (f *File) SetDouble(group, key string, value float64) error {
	if key == "" {
		return ErrEmptyKey
	}
	s := strconv.FormatFloat(value, 'g', -1, 64)
	return f.setKeyValue(stripBrackets(group), key, func(e *entry) { e.value = &s })
}

// SetString sets the string value of group/key.
func (f *File) SetString(group, key, value string) error {
	if key == "" {
		return ErrEmptyKey
	}
	return f.setKeyValue(stripBrackets(group), key, func(e *entry) { e.value = &value })
}

// SetBool sets the bool value of group/key.
func (f *File) SetBool(group, key string, value bool) error {
	if key == "" {
		return ErrEmptyKey
	}
	s := "false"
	if value {
		s = "true"
	}
	return f.setKeyValue(stripBrackets(group), key, func(e *entry) { e.value = &s })
}

// SetQuotes sets whether the value of group/key is enclosed by quotes when
// f is written to a file. Unlike the other setters, it does not create the
// entry if group/key does not already exist.
func (f *File) SetQuotes(group, key string, quotes bool) error {
	if key == "" {
		return ErrEmptyKey
	}
	num, err := f.findKey(stripBrackets(group), key)
	if err != nil {
		return err
	}
	f.entries[num].quotes = quotes
	return nil
}
