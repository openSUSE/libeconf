package econf

/*
// This preamble must only contain declarations, not definitions. Cgo copies
// it into both the normal per-file C translation unit and the generated
// export header; a function body here would be compiled twice and fail to
// link with "multiple definition of ...". The actual trampoline that C code
// calls (which forwards into goEconfCallback below) lives in trampoline.go
// instead, which has no //export comment of its own.
#include <stdbool.h>
#include <stdlib.h>
#include <libeconf.h>
*/
import "C"

import (
	"sync"
	"unsafe"
)

// FileCallback is invoked once per candidate configuration file while
// reading. Return true to allow parsing that file, false to abort parsing
// (the read call then fails with ErrParsingCallbackFailed).
type FileCallback func(filename string) bool

var (
	callbackMu    sync.Mutex
	callbackTable = map[unsafe.Pointer]FileCallback{}
)

// registerCallback stores fn under a freshly allocated, unique token. The
// token is passed to the C API as callback_data; the returned cleanup func
// must be deferred by the caller to unregister and free the token.
func registerCallback(fn FileCallback) (unsafe.Pointer, func()) {
	token := C.malloc(1)
	callbackMu.Lock()
	callbackTable[token] = fn
	callbackMu.Unlock()
	return token, func() {
		callbackMu.Lock()
		delete(callbackTable, token)
		callbackMu.Unlock()
		C.free(token)
	}
}

//export goEconfCallback
func goEconfCallback(filename *C.char, data unsafe.Pointer) C.bool {
	callbackMu.Lock()
	fn := callbackTable[data]
	callbackMu.Unlock()
	if fn == nil {
		// A nil FileCallback means "no filtering", matching the
		// native package's "if callback != nil && !callback(...)" check:
		// every candidate file is accepted.
		return C.bool(true)
	}
	return C.bool(fn(C.GoString(filename)))
}
