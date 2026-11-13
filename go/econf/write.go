package econf

import (
	"fmt"
	"os"
	"path/filepath"
	"strings"
)

// WriteFile writes f's contents to saveToDir/fileName, mirroring
// econf_writeFile's output formatting byte for byte (modulo the C
// implementation's incidental BUFSIZ comment-truncation limit, which is
// not replicated here).
func (f *File) WriteFile(saveToDir, fileName string) error {
	info, err := os.Stat(saveToDir)
	if err != nil {
		if os.IsNotExist(err) {
			return ErrNoFile
		}
		return ErrError
	}
	if !info.IsDir() {
		return ErrNoFile
	}

	out, err := os.Create(filepath.Join(saveToDir, fileName))
	if err != nil {
		return ErrWriteError
	}
	defer out.Close()

	for i, e := range f.entries {
		if i == 0 || f.entries[i-1].group != e.group {
			if i != 0 {
				fmt.Fprint(out, "\n")
			}
			if e.group != noGroup {
				fmt.Fprintf(out, "%s\n", addBrackets(e.group))
			}
		}

		if e.commentBeforeKey != nil && *e.commentBeforeKey != "" {
			for _, line := range strings.Split(*e.commentBeforeKey, "\n") {
				fmt.Fprintf(out, "%c%s\n", f.comment, line)
			}
		}

		fmt.Fprintf(out, "%s%c", e.key, f.delimiter)
		if e.value != nil {
			if e.quotes {
				fmt.Fprintf(out, "\"%s\"", *e.value)
			} else {
				fmt.Fprint(out, *e.value)
			}
		}

		if e.commentAfterValue != nil && *e.commentAfterValue != "" {
			for _, line := range strings.Split(*e.commentAfterValue, "\n") {
				fmt.Fprintf(out, " %c%s\n", f.comment, line)
			}
		}
		fmt.Fprint(out, "\n")
	}

	return nil
}
