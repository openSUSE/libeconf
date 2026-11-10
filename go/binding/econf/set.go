package econf

/*
#include <stdlib.h>
#include <libeconf.h>
*/
import "C"

import "unsafe"

// SetInt sets the int32 value of group/key.
func (f *File) SetInt(group, key string, value int32) error {
	cGroup, freeGroup := optionalCString(group)
	defer freeGroup()
	cKey := C.CString(key)
	defer C.free(unsafe.Pointer(cKey))

	return newError(C.econf_setIntValue(f.ptr, cGroup, cKey, C.int32_t(value)))
}

// SetInt64 sets the int64 value of group/key.
func (f *File) SetInt64(group, key string, value int64) error {
	cGroup, freeGroup := optionalCString(group)
	defer freeGroup()
	cKey := C.CString(key)
	defer C.free(unsafe.Pointer(cKey))

	return newError(C.econf_setInt64Value(f.ptr, cGroup, cKey, C.int64_t(value)))
}

// SetUInt sets the uint32 value of group/key.
func (f *File) SetUInt(group, key string, value uint32) error {
	cGroup, freeGroup := optionalCString(group)
	defer freeGroup()
	cKey := C.CString(key)
	defer C.free(unsafe.Pointer(cKey))

	return newError(C.econf_setUIntValue(f.ptr, cGroup, cKey, C.uint32_t(value)))
}

// SetUInt64 sets the uint64 value of group/key.
func (f *File) SetUInt64(group, key string, value uint64) error {
	cGroup, freeGroup := optionalCString(group)
	defer freeGroup()
	cKey := C.CString(key)
	defer C.free(unsafe.Pointer(cKey))

	return newError(C.econf_setUInt64Value(f.ptr, cGroup, cKey, C.uint64_t(value)))
}

// SetFloat sets the float32 value of group/key.
func (f *File) SetFloat(group, key string, value float32) error {
	cGroup, freeGroup := optionalCString(group)
	defer freeGroup()
	cKey := C.CString(key)
	defer C.free(unsafe.Pointer(cKey))

	return newError(C.econf_setFloatValue(f.ptr, cGroup, cKey, C.float(value)))
}

// SetDouble sets the float64 value of group/key.
func (f *File) SetDouble(group, key string, value float64) error {
	cGroup, freeGroup := optionalCString(group)
	defer freeGroup()
	cKey := C.CString(key)
	defer C.free(unsafe.Pointer(cKey))

	return newError(C.econf_setDoubleValue(f.ptr, cGroup, cKey, C.double(value)))
}

// SetString sets the string value of group/key.
func (f *File) SetString(group, key, value string) error {
	cGroup, freeGroup := optionalCString(group)
	defer freeGroup()
	cKey := C.CString(key)
	defer C.free(unsafe.Pointer(cKey))
	cValue := C.CString(value)
	defer C.free(unsafe.Pointer(cValue))

	return newError(C.econf_setStringValue(f.ptr, cGroup, cKey, cValue))
}

// SetBool sets the bool value of group/key.
func (f *File) SetBool(group, key string, value bool) error {
	cGroup, freeGroup := optionalCString(group)
	defer freeGroup()
	cKey := C.CString(key)
	defer C.free(unsafe.Pointer(cKey))

	boolStr := "false"
	if value {
		boolStr = "true"
	}
	cValue := C.CString(boolStr)
	defer C.free(unsafe.Pointer(cValue))

	return newError(C.econf_setBoolValue(f.ptr, cGroup, cKey, cValue))
}

// SetQuotes sets whether the value of group/key is enclosed by quotes when
// f is written to a file.
func (f *File) SetQuotes(group, key string, quotes bool) error {
	cGroup, freeGroup := optionalCString(group)
	defer freeGroup()
	cKey := C.CString(key)
	defer C.free(unsafe.Pointer(cKey))

	return newError(C.econf_setQuotes(f.ptr, cGroup, cKey, C.bool(quotes)))
}
