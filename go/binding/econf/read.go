package econf

/*
#include <stdbool.h>
#include <stdlib.h>
#include <libeconf.h>

extern bool econf_callback_trampoline(const char *filename, const void *data);
*/
import "C"

import "unsafe"

// ReadFile parses fileName and returns its contents.
func ReadFile(fileName, delim, comment string) (*File, error) {
	cFileName := C.CString(fileName)
	defer C.free(unsafe.Pointer(cFileName))
	cDelim := C.CString(delim)
	defer C.free(unsafe.Pointer(cDelim))
	cComment := C.CString(comment)
	defer C.free(unsafe.Pointer(cComment))

	var ptr *C.econf_file
	if err := newError(C.econf_readFile(&ptr, cFileName, cDelim, cComment)); err != nil {
		return nil, err
	}
	return newFile(ptr), nil
}

// ReadFileWithCallback parses fileName and returns its contents. callback is
// invoked with the path before parsing; returning false aborts the read
// with ErrParsingCallbackFailed.
func ReadFileWithCallback(fileName, delim, comment string, callback FileCallback) (*File, error) {
	cFileName := C.CString(fileName)
	defer C.free(unsafe.Pointer(cFileName))
	cDelim := C.CString(delim)
	defer C.free(unsafe.Pointer(cDelim))
	cComment := C.CString(comment)
	defer C.free(unsafe.Pointer(cComment))

	token, cleanup := registerCallback(callback)
	defer cleanup()

	var ptr *C.econf_file
	err := newError(C.econf_readFileWithCallback(&ptr, cFileName, cDelim, cComment,
		(*[0]byte)(C.econf_callback_trampoline), token))
	if err != nil {
		return nil, err
	}
	return newFile(ptr), nil
}

// MergeFiles merges etcFile on top of usrFile; entries in etcFile win.
func MergeFiles(usrFile, etcFile *File) (*File, error) {
	var ptr *C.econf_file
	if err := newError(C.econf_mergeFiles(&ptr, usrFile.ptr, etcFile.ptr)); err != nil {
		return nil, err
	}
	return newFile(ptr), nil
}

// ReadConfig evaluates key/values of a configuration by reading and merging
// all matching files from the vendor, runtime and admin directories. project,
// configName and configSuffix may be "" to mean "not set" (see
// econf_readConfig in include/libeconf.h for the exact directory/precedence
// rules).
func ReadConfig(project, usrSubdir, configName, configSuffix, delim, comment string) (*File, error) {
	cProject, freeProject := optionalCString(project)
	defer freeProject()
	cUsrSubdir := C.CString(usrSubdir)
	defer C.free(unsafe.Pointer(cUsrSubdir))
	cConfigName, freeConfigName := optionalCString(configName)
	defer freeConfigName()
	cConfigSuffix, freeConfigSuffix := optionalCString(configSuffix)
	defer freeConfigSuffix()
	cDelim := C.CString(delim)
	defer C.free(unsafe.Pointer(cDelim))
	cComment := C.CString(comment)
	defer C.free(unsafe.Pointer(cComment))

	var ptr *C.econf_file
	err := newError(C.econf_readConfig(&ptr, cProject, cUsrSubdir, cConfigName, cConfigSuffix, cDelim, cComment))
	if err != nil {
		return nil, err
	}
	return newFile(ptr), nil
}

// ReadConfigWithCallback behaves like ReadConfig, additionally invoking
// callback for every candidate file before it is parsed.
func ReadConfigWithCallback(project, usrSubdir, configName, configSuffix, delim, comment string, callback FileCallback) (*File, error) {
	cProject, freeProject := optionalCString(project)
	defer freeProject()
	cUsrSubdir := C.CString(usrSubdir)
	defer C.free(unsafe.Pointer(cUsrSubdir))
	cConfigName, freeConfigName := optionalCString(configName)
	defer freeConfigName()
	cConfigSuffix, freeConfigSuffix := optionalCString(configSuffix)
	defer freeConfigSuffix()
	cDelim := C.CString(delim)
	defer C.free(unsafe.Pointer(cDelim))
	cComment := C.CString(comment)
	defer C.free(unsafe.Pointer(cComment))

	token, cleanup := registerCallback(callback)
	defer cleanup()

	var ptr *C.econf_file
	err := newError(C.econf_readConfigWithCallback(&ptr, cProject, cUsrSubdir, cConfigName, cConfigSuffix,
		cDelim, cComment, (*[0]byte)(C.econf_callback_trampoline), token))
	if err != nil {
		return nil, err
	}
	return newFile(ptr), nil
}

// ReadDirsHistory evaluates key/values for every configuration file found in
// usrConfDir and etcConfDir and returns one *File per file found, in the
// order they were read (lowest to highest precedence).
func ReadDirsHistory(usrConfDir, etcConfDir, configName, configSuffix, delim, comment string) ([]*File, error) {
	cUsrConfDir := C.CString(usrConfDir)
	defer C.free(unsafe.Pointer(cUsrConfDir))
	cEtcConfDir := C.CString(etcConfDir)
	defer C.free(unsafe.Pointer(cEtcConfDir))
	cConfigName := C.CString(configName)
	defer C.free(unsafe.Pointer(cConfigName))
	cConfigSuffix, freeConfigSuffix := optionalCString(configSuffix)
	defer freeConfigSuffix()
	cDelim := C.CString(delim)
	defer C.free(unsafe.Pointer(cDelim))
	cComment := C.CString(comment)
	defer C.free(unsafe.Pointer(cComment))

	var cFiles **C.econf_file
	var size C.size_t
	err := newError(C.econf_readDirsHistory(&cFiles, &size, cUsrConfDir, cEtcConfDir, cConfigName, cConfigSuffix, cDelim, cComment))
	if err != nil {
		return nil, err
	}
	return wrapFileArray(cFiles, size), nil
}

// ReadDirsHistoryWithCallback behaves like ReadDirsHistory, additionally
// invoking callback for every candidate file before it is parsed.
func ReadDirsHistoryWithCallback(usrConfDir, etcConfDir, configName, configSuffix, delim, comment string, callback FileCallback) ([]*File, error) {
	cUsrConfDir := C.CString(usrConfDir)
	defer C.free(unsafe.Pointer(cUsrConfDir))
	cEtcConfDir := C.CString(etcConfDir)
	defer C.free(unsafe.Pointer(cEtcConfDir))
	cConfigName := C.CString(configName)
	defer C.free(unsafe.Pointer(cConfigName))
	cConfigSuffix, freeConfigSuffix := optionalCString(configSuffix)
	defer freeConfigSuffix()
	cDelim := C.CString(delim)
	defer C.free(unsafe.Pointer(cDelim))
	cComment := C.CString(comment)
	defer C.free(unsafe.Pointer(cComment))

	token, cleanup := registerCallback(callback)
	defer cleanup()

	var cFiles **C.econf_file
	var size C.size_t
	err := newError(C.econf_readDirsHistoryWithCallback(&cFiles, &size, cUsrConfDir, cEtcConfDir, cConfigName,
		cConfigSuffix, cDelim, cComment, (*[0]byte)(C.econf_callback_trampoline), token))
	if err != nil {
		return nil, err
	}
	return wrapFileArray(cFiles, size), nil
}

// wrapFileArray wraps each of the size econf_file* in cFiles into a *File
// and frees the (plain malloc'd) array container itself. Ownership of each
// individual econf_file transfers to the returned *File values.
func wrapFileArray(cFiles **C.econf_file, size C.size_t) []*File {
	if size == 0 {
		return nil
	}
	cSlice := unsafe.Slice(cFiles, int(size))
	files := make([]*File, int(size))
	for i, ptr := range cSlice {
		files[i] = newFile(ptr)
	}
	C.free(unsafe.Pointer(cFiles))
	return files
}
