package econf

/*
#include <stdlib.h>
#include <libeconf.h>
*/
import "C"

import "unsafe"

// Int returns the int32 value of group/key. group may be "" if the key is
// not inside a group.
func (f *File) Int(group, key string) (int32, error) {
	cGroup, freeGroup := optionalCString(group)
	defer freeGroup()
	cKey := C.CString(key)
	defer C.free(unsafe.Pointer(cKey))

	var result C.int32_t
	err := newError(C.econf_getIntValue(f.ptr, cGroup, cKey, &result))
	return int32(result), err
}

// IntDef returns the int32 value of group/key, or def if the key is not
// found.
func (f *File) IntDef(group, key string, def int32) (int32, error) {
	cGroup, freeGroup := optionalCString(group)
	defer freeGroup()
	cKey := C.CString(key)
	defer C.free(unsafe.Pointer(cKey))

	var result C.int32_t
	err := newError(C.econf_getIntValueDef(f.ptr, cGroup, cKey, &result, C.int32_t(def)))
	return int32(result), err
}

// Int64 returns the int64 value of group/key.
func (f *File) Int64(group, key string) (int64, error) {
	cGroup, freeGroup := optionalCString(group)
	defer freeGroup()
	cKey := C.CString(key)
	defer C.free(unsafe.Pointer(cKey))

	var result C.int64_t
	err := newError(C.econf_getInt64Value(f.ptr, cGroup, cKey, &result))
	return int64(result), err
}

// Int64Def returns the int64 value of group/key, or def if the key is not
// found.
func (f *File) Int64Def(group, key string, def int64) (int64, error) {
	cGroup, freeGroup := optionalCString(group)
	defer freeGroup()
	cKey := C.CString(key)
	defer C.free(unsafe.Pointer(cKey))

	var result C.int64_t
	err := newError(C.econf_getInt64ValueDef(f.ptr, cGroup, cKey, &result, C.int64_t(def)))
	return int64(result), err
}

// UInt returns the uint32 value of group/key.
func (f *File) UInt(group, key string) (uint32, error) {
	cGroup, freeGroup := optionalCString(group)
	defer freeGroup()
	cKey := C.CString(key)
	defer C.free(unsafe.Pointer(cKey))

	var result C.uint32_t
	err := newError(C.econf_getUIntValue(f.ptr, cGroup, cKey, &result))
	return uint32(result), err
}

// UIntDef returns the uint32 value of group/key, or def if the key is not
// found.
func (f *File) UIntDef(group, key string, def uint32) (uint32, error) {
	cGroup, freeGroup := optionalCString(group)
	defer freeGroup()
	cKey := C.CString(key)
	defer C.free(unsafe.Pointer(cKey))

	var result C.uint32_t
	err := newError(C.econf_getUIntValueDef(f.ptr, cGroup, cKey, &result, C.uint32_t(def)))
	return uint32(result), err
}

// UInt64 returns the uint64 value of group/key.
func (f *File) UInt64(group, key string) (uint64, error) {
	cGroup, freeGroup := optionalCString(group)
	defer freeGroup()
	cKey := C.CString(key)
	defer C.free(unsafe.Pointer(cKey))

	var result C.uint64_t
	err := newError(C.econf_getUInt64Value(f.ptr, cGroup, cKey, &result))
	return uint64(result), err
}

// UInt64Def returns the uint64 value of group/key, or def if the key is not
// found.
func (f *File) UInt64Def(group, key string, def uint64) (uint64, error) {
	cGroup, freeGroup := optionalCString(group)
	defer freeGroup()
	cKey := C.CString(key)
	defer C.free(unsafe.Pointer(cKey))

	var result C.uint64_t
	err := newError(C.econf_getUInt64ValueDef(f.ptr, cGroup, cKey, &result, C.uint64_t(def)))
	return uint64(result), err
}

// Float returns the float32 value of group/key.
func (f *File) Float(group, key string) (float32, error) {
	cGroup, freeGroup := optionalCString(group)
	defer freeGroup()
	cKey := C.CString(key)
	defer C.free(unsafe.Pointer(cKey))

	var result C.float
	err := newError(C.econf_getFloatValue(f.ptr, cGroup, cKey, &result))
	return float32(result), err
}

// FloatDef returns the float32 value of group/key, or def if the key is not
// found.
func (f *File) FloatDef(group, key string, def float32) (float32, error) {
	cGroup, freeGroup := optionalCString(group)
	defer freeGroup()
	cKey := C.CString(key)
	defer C.free(unsafe.Pointer(cKey))

	var result C.float
	err := newError(C.econf_getFloatValueDef(f.ptr, cGroup, cKey, &result, C.float(def)))
	return float32(result), err
}

// Double returns the float64 value of group/key.
func (f *File) Double(group, key string) (float64, error) {
	cGroup, freeGroup := optionalCString(group)
	defer freeGroup()
	cKey := C.CString(key)
	defer C.free(unsafe.Pointer(cKey))

	var result C.double
	err := newError(C.econf_getDoubleValue(f.ptr, cGroup, cKey, &result))
	return float64(result), err
}

// DoubleDef returns the float64 value of group/key, or def if the key is
// not found.
func (f *File) DoubleDef(group, key string, def float64) (float64, error) {
	cGroup, freeGroup := optionalCString(group)
	defer freeGroup()
	cKey := C.CString(key)
	defer C.free(unsafe.Pointer(cKey))

	var result C.double
	err := newError(C.econf_getDoubleValueDef(f.ptr, cGroup, cKey, &result, C.double(def)))
	return float64(result), err
}

// String returns the string value of group/key.
func (f *File) String(group, key string) (string, error) {
	cGroup, freeGroup := optionalCString(group)
	defer freeGroup()
	cKey := C.CString(key)
	defer C.free(unsafe.Pointer(cKey))

	var result *C.char
	if err := newError(C.econf_getStringValue(f.ptr, cGroup, cKey, &result)); err != nil {
		return "", err
	}
	defer C.free(unsafe.Pointer(result))
	return C.GoString(result), nil
}

// StringDef returns the string value of group/key, or def if the key is not
// found.
func (f *File) StringDef(group, key, def string) (string, error) {
	cGroup, freeGroup := optionalCString(group)
	defer freeGroup()
	cKey := C.CString(key)
	defer C.free(unsafe.Pointer(cKey))
	cDef := C.CString(def)
	defer C.free(unsafe.Pointer(cDef))

	var result *C.char
	err := newError(C.econf_getStringValueDef(f.ptr, cGroup, cKey, &result, cDef))
	if result == nil {
		return "", err
	}
	defer C.free(unsafe.Pointer(result))
	return C.GoString(result), err
}

// Bool returns the bool value of group/key.
func (f *File) Bool(group, key string) (bool, error) {
	cGroup, freeGroup := optionalCString(group)
	defer freeGroup()
	cKey := C.CString(key)
	defer C.free(unsafe.Pointer(cKey))

	var result C.bool
	err := newError(C.econf_getBoolValue(f.ptr, cGroup, cKey, &result))
	return bool(result), err
}

// BoolDef returns the bool value of group/key, or def if the key is not
// found.
func (f *File) BoolDef(group, key string, def bool) (bool, error) {
	cGroup, freeGroup := optionalCString(group)
	defer freeGroup()
	cKey := C.CString(key)
	defer C.free(unsafe.Pointer(cKey))

	var result C.bool
	err := newError(C.econf_getBoolValueDef(f.ptr, cGroup, cKey, &result, C.bool(def)))
	return bool(result), err
}

// HasQuotes reports whether the value of group/key is enclosed by quotes.
func (f *File) HasQuotes(group, key string) bool {
	cGroup, freeGroup := optionalCString(group)
	defer freeGroup()
	cKey := C.CString(key)
	defer C.free(unsafe.Pointer(cKey))

	return bool(C.econf_hasQuotes(f.ptr, cGroup, cKey))
}
