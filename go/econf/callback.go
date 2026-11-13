package econf

// FileCallback is invoked once per candidate configuration file while
// reading. Return true to allow parsing that file, false to abort parsing
// (the read call then fails with ErrParsingCallbackFailed).
type FileCallback func(filename string) bool
