// Package econf — see doc.go for the package-level overview.
package econf

/*
#cgo pkg-config: libeconf
#include <stdlib.h>
#include <libeconf.h>
*/
import "C"

import (
	"runtime"
	"sync/atomic"
	"unsafe"
)

// File wraps an econf_file object: the in-memory representation of a parsed
// and/or merged configuration.
type File struct {
	ptr    *C.econf_file
	closed atomic.Bool
}

func newFile(ptr *C.econf_file) *File {
	f := &File{ptr: ptr}
	runtime.SetFinalizer(f, (*File).Close)
	return f
}

// Close releases the C memory backing f. It is safe to call multiple times
// and on a nil *File. f must not be used again afterwards.
func (f *File) Close() error {
	if f == nil || f.ptr == nil {
		return nil
	}
	if !f.closed.CompareAndSwap(false, true) {
		return nil
	}
	runtime.SetFinalizer(f, nil)
	C.econf_freeFile(f.ptr)
	f.ptr = nil
	return nil
}

// NewKeyFile creates a new, empty econf_file with the given delimiter and
// comment characters.
func NewKeyFile(delimiter, comment byte) (*File, error) {
	var ptr *C.econf_file
	if err := newError(C.econf_newKeyFile(&ptr, C.char(delimiter), C.char(comment))); err != nil {
		return nil, err
	}
	return newFile(ptr), nil
}

// NewKeyFileWithOptions creates a new, empty econf_file configured by a
// ";"-separated "key=value" options string (e.g. "JOIN_SAME_ENTRIES=1").
// See econf_newKeyFile_with_options in include/libeconf.h for the full list
// of supported options.
func NewKeyFileWithOptions(options string) (*File, error) {
	cOptions := C.CString(options)
	defer C.free(unsafe.Pointer(cOptions))

	var ptr *C.econf_file
	if err := newError(C.econf_newKeyFile_with_options(&ptr, cOptions)); err != nil {
		return nil, err
	}
	return newFile(ptr), nil
}

// NewIniFile creates a new econf_file in INI format ("=" delimiter, "#"
// comments).
func NewIniFile() (*File, error) {
	var ptr *C.econf_file
	if err := newError(C.econf_newIniFile(&ptr)); err != nil {
		return nil, err
	}
	return newFile(ptr), nil
}

// Path returns the absolute path of the parsed file, or an empty string for
// already-merged data.
func (f *File) Path() string {
	cPath := C.econf_getPath(f.ptr)
	if cPath == nil {
		return ""
	}
	defer C.free(unsafe.Pointer(cPath))
	return C.GoString(cPath)
}

// Groups returns all group names found in f.
func (f *File) Groups() ([]string, error) {
	var length C.size_t
	var groups **C.char
	if err := newError(C.econf_getGroups(f.ptr, &length, &groups)); err != nil {
		return nil, err
	}
	defer C.econf_freeArray(groups)
	return cStringArrayToGo(groups, length), nil
}

// Keys returns all keys in the given group. If group is "", keys from all
// groups are returned.
func (f *File) Keys(group string) ([]string, error) {
	cGroup, free := optionalCString(group)
	defer free()

	var length C.size_t
	var keys **C.char
	if err := newError(C.econf_getKeys(f.ptr, cGroup, &length, &keys)); err != nil {
		return nil, err
	}
	defer C.econf_freeArray(keys)
	return cStringArrayToGo(keys, length), nil
}

// CommentTag returns the comment character tag of f.
func (f *File) CommentTag() byte {
	return byte(C.econf_comment_tag(f.ptr))
}

// SetCommentTag sets the comment character tag of f.
func (f *File) SetCommentTag(comment byte) {
	C.econf_set_comment_tag(f.ptr, C.char(comment))
}

// DelimiterTag returns the key/value delimiter character of f.
func (f *File) DelimiterTag() byte {
	return byte(C.econf_delimiter_tag(f.ptr))
}

// SetDelimiterTag sets the key/value delimiter character of f.
func (f *File) SetDelimiterTag(delimiter byte) {
	C.econf_set_delimiter_tag(f.ptr, C.char(delimiter))
}

// WriteFile writes f's contents to saveToDir/fileName.
func (f *File) WriteFile(saveToDir, fileName string) error {
	cDir := C.CString(saveToDir)
	defer C.free(unsafe.Pointer(cDir))
	cName := C.CString(fileName)
	defer C.free(unsafe.Pointer(cName))

	return newError(C.econf_writeFile(f.ptr, cDir, cName))
}

// optionalCString allocates a C string for s, unless s is empty in which
// case it returns NULL — the convention the C API uses for "no group"/"all
// keys". The returned free func must always be called.
func optionalCString(s string) (*C.char, func()) {
	if s == "" {
		return nil, func() {}
	}
	cs := C.CString(s)
	return cs, func() { C.free(unsafe.Pointer(cs)) }
}

// cStringArrayToGo copies a NULL-terminated-length C string array into a Go
// slice. It does not free arr; the caller remains responsible for that.
func cStringArrayToGo(arr **C.char, length C.size_t) []string {
	if length == 0 {
		return nil
	}
	cSlice := unsafe.Slice(arr, int(length))
	result := make([]string, int(length))
	for i, cs := range cSlice {
		result[i] = C.GoString(cs)
	}
	return result
}
