package econf

import (
	"os"
	"path/filepath"
	"strings"
)

// checkDelim mirrors getfilecontents.c's check_delim.
func checkDelim(delim string) (hasWsp, hasNonWsp bool) {
	for i := 0; i < len(delim); i++ {
		if isSpaceC(delim[i]) {
			hasWsp = true
		} else {
			hasNonWsp = true
		}
		if hasWsp && hasNonWsp {
			break
		}
	}
	return
}

// getAbsolutePath mirrors helpers.c's get_absolute_path: absolute inputs
// are returned as-is, relative inputs are resolved (and must exist).
func getAbsolutePath(path string) (string, error) {
	if strings.HasPrefix(path, "/") {
		return path, nil
	}
	abs, err := filepath.Abs(path)
	if err != nil {
		return "", ErrNoFile
	}
	resolved, err := filepath.EvalSymlinks(abs)
	if err != nil {
		return "", ErrNoFile
	}
	return resolved, nil
}

// splitRawLines splits file content the way a getline()-based reader would
// see it: trailing newlines don't produce a synthetic extra empty line, and
// a completely empty file has zero lines.
func splitRawLines(content string) []string {
	if content == "" {
		return nil
	}
	lines := strings.Split(content, "\n")
	if strings.HasSuffix(content, "\n") {
		lines = lines[:len(lines)-1]
	}
	return lines
}

func appendComment(existing *string, addition string) *string {
	if existing != nil {
		s := *existing + "\n" + addition
		return &s
	}
	s := addition
	return &s
}

// findNextCommentChar mirrors the inner re-scan loop in getfilecontents.c's
// read_file: it tries each character of comment in order (not leftmost
// occurrence across all of them), returning the first match found at or
// after from.
func findNextCommentChar(s string, from int, comment string) int {
	if from < 0 || from > len(s) {
		return -1
	}
	for k := 0; k < len(comment); k++ {
		if idx := strings.IndexByte(s[from:], comment[k]); idx >= 0 {
			return from + idx
		}
	}
	return -1
}

// readFile parses path into f, mirroring getfilecontents.c's read_file
// (permission/ownership/symlink checks from read_file_with_callback are
// intentionally not ported; see go/econf doc.go).
func (f *File) readFile(path, delim, comment string) error {
	if comment == "" {
		comment = "#"
	}

	absPath, err := getAbsolutePath(path)
	if err != nil {
		return err
	}

	data, err := os.ReadFile(absPath)
	if err != nil {
		return ErrNoFile
	}

	hasWsp, hasNonWsp := checkDelim(delim)

	f.path = absPath
	if len(delim) > 0 {
		f.delimiter = delim[0]
	} else {
		f.delimiter = 0
	}
	f.comment = comment[0]

	lines := splitRawLines(string(data))

	var currentGroup string
	var currentCommentBeforeKey *string
	var currentCommentAfterValue *string

	resetComments := func() {
		currentCommentBeforeKey = nil
		currentCommentAfterValue = nil
	}

	for idx, rawLine := range lines {
		line := uint64(idx + 1)
		if rawLine == "" {
			continue
		}

		buf := rawLine
		nameStart := 0
		for nameStart < len(buf) && isSpaceC(buf[nameStart]) {
			nameStart++
		}

		// dataSideEffect mirrors a quirk of the C scanner: computing the
		// trailing-comment-vs-quote heuristic below has the side effect of
		// advancing a shared "data" pointer to the first whitespace (or
		// delimiter) character in the line. In no-delimiter mode that
		// stale pointer - not NULL - ends up as the stored value whenever
		// a line has a genuine trailing comment, so real-world no-
		// delimiter files (e.g. a flat list of shells with trailing
		// comments) observably get a non-nil value in that case.
		dataSideEffect := -1

		// Scan for comments, mirroring the per-comment-character loop in
		// read_file.
		for ci := 0; ci < len(comment); ci++ {
			name := buf[nameStart:]
			rel := strings.IndexByte(name, comment[ci])
			if rel < 0 {
				continue
			}
			p := nameStart + rel

			if p == nameStart {
				// Comment is the entire (trimmed) line: a before-key comment.
				currentCommentBeforeKey = appendComment(currentCommentBeforeKey, buf[p+1:])
				buf = buf[:p]
				continue
			}

			if f.pythonStyle {
				continue
			}

			name2 := buf[nameStart:]
			firstQuote := strings.IndexByte(name2, '"')
			lastQuote := strings.LastIndexByte(name2, '"')

			dj := 0
			for dj < len(name2) && !(isSpaceC(name2[dj]) || strings.IndexByte(delim, name2[dj]) >= 0) {
				dj++
			}
			dataSideEffect = nameStart + dj

			absFirstQuote, absLastQuote := -1, -1
			if firstQuote >= 0 {
				absFirstQuote = nameStart + firstQuote
			}
			if lastQuote >= 0 {
				absLastQuote = nameStart + lastQuote
			}
			// delim_found is computed in the C source but the branch that
			// uses it always forces it back to false; kept as a literal
			// constant here for fidelity with that (apparently dead) logic.
			const delimFound = false

			pp := p
			for pp >= 0 {
				isReal := absFirstQuote == -1 ||
					(absFirstQuote != absLastQuote && absLastQuote < pp) ||
					(absFirstQuote == absLastQuote && absLastQuote < pp && !delimFound)
				if isReal {
					currentCommentAfterValue = appendComment(currentCommentAfterValue, buf[pp+1:])
					buf = buf[:pp]
					pp = findNextCommentChar(buf, pp, comment)
				} else {
					pp = findNextCommentChar(buf, pp+1, comment)
				}
			}
		}

		if buf == "" {
			continue
		}

		name := buf[nameStart:]

		// Group header.
		if len(name) > 0 && name[0] == '[' {
			inner := name[1:]
			end := len(inner)
			for end > 0 && isSpaceC(inner[end-1]) {
				end--
			}
			if end == 0 || inner[end-1] != ']' {
				if strings.Contains(inner, "]") {
					return ErrTextAfterSection
				}
				return ErrMissingBracket
			}
			sectionName := inner[:end-1]
			if sectionName == "" {
				return ErrEmptySectionName
			}
			currentGroup = sectionName
			f.internGroup(sectionName)
			continue
		}

		if delim == "" || delim == "\n" {
			// No delimiter configured: store the (trimmed) name as a key.
			// See dataSideEffect's declaration above for why this isn't
			// unconditionally a nil value.
			var noDelimValue *string
			if dataSideEffect >= 0 {
				from := dataSideEffect
				if from > len(buf) {
					from = len(buf)
				}
				v := buf[from:]
				noDelimValue = &v
			}
			if err := f.store(currentGroup, name, noDelimValue, line, currentCommentBeforeKey, currentCommentAfterValue, false, false); err != nil {
				return err
			}
			resetComments()
			continue
		}

		// A delimiter is configured: find the end of the key.
		keyEnd := nameStart
		for keyEnd < len(buf) && !(isSpaceC(buf[keyEnd]) || strings.IndexByte(delim, buf[keyEnd]) >= 0) {
			keyEnd++
		}
		dataPos := keyEnd
		delimSeen := false
		if keyEnd > nameStart && dataPos < len(buf) {
			if hasWsp && hasNonWsp {
				delimSeen = !isSpaceC(buf[dataPos]) && strings.IndexByte(delim, buf[dataPos]) >= 0
			} else {
				delimSeen = strings.IndexByte(delim, buf[dataPos]) >= 0
			}
			dataPos++
		}
		key := buf[nameStart:keyEnd]

		if !hasWsp || !hasNonWsp {
			foundDelim := false
			if !f.pythonStyle || len(rawLine) == 0 || !isSpaceC(rawLine[0]) {
				foundDelim = delimSeen
				if !foundDelim {
					c := dataPos
					for c < len(buf) && strings.IndexByte(delim, buf[c]) < 0 {
						c++
					}
					if c < len(buf) {
						foundDelim = true
					}
				}
			}
			if !foundDelim && len(f.entries) > 0 && f.entries[len(f.entries)-1].lineNumber+1 == line {
				appendVal := rawLine
				if !f.pythonStyle {
					cut := len(appendVal)
					for ci := 0; ci < len(comment); ci++ {
						if i2 := strings.IndexByte(appendVal, comment[ci]); i2 >= 0 && i2 < cut {
							cut = i2
						}
					}
					appendVal = appendVal[:cut]
				}
				if err := f.store(currentGroup, key, &appendVal, line, currentCommentBeforeKey, currentCommentAfterValue, false, true); err != nil {
					return err
				}
				resetComments()
				continue
			}
		}

		if key == "" {
			continue
		}

		var value *string
		quoteSeen := false
		if dataPos >= len(buf) {
			value = nil
		} else {
			vstart := dataPos
			for vstart < len(buf) && isSpaceC(buf[vstart]) {
				vstart++
			}
			if !hasWsp && !delimSeen {
				if vstart >= len(buf) || strings.IndexByte(delim, buf[vstart]) < 0 {
					return ErrMissingDelimiter
				}
				vstart++
				for vstart < len(buf) && isSpaceC(buf[vstart]) {
					vstart++
				}
			} else if hasWsp && hasNonWsp && !delimSeen && vstart < len(buf) && strings.IndexByte(delim, buf[vstart]) >= 0 {
				vstart++
				for vstart < len(buf) && isSpaceC(buf[vstart]) {
					vstart++
				}
			}

			if vstart < len(buf) && buf[vstart] == '"' {
				quoteSeen = true
				vstart++
			}

			pIdx := len(buf)
			if pIdx > vstart {
				pIdx--
			}
			for pIdx > vstart && isSpaceC(buf[pIdx]) {
				pIdx--
			}
			if quoteSeen {
				if pIdx < len(buf) && buf[pIdx] == '"' {
					pIdx--
				} else if vstart > 0 {
					vstart--
				}
			}

			var v string
			if pIdx >= vstart && pIdx < len(buf) {
				v = buf[vstart : pIdx+1]
			}
			value = &v
		}

		if err := f.store(currentGroup, key, value, line, currentCommentBeforeKey, currentCommentAfterValue, quoteSeen, false); err != nil {
			return err
		}
		resetComments()
	}

	if f.joinSameEntries {
		joinSameEntries(f)
	}

	return nil
}

// store mirrors getfilecontents.c's store: appendEntry=true extends the
// most recently added entry's value (multiline continuation); otherwise a
// brand-new entry is appended.
func (f *File) store(group, key string, value *string, line uint64, commentBefore, commentAfter *string, quotes bool, appendEntry bool) error {
	if appendEntry {
		if len(f.entries) == 0 {
			return ErrMissingDelimiter
		}
		v := valueOf(value)
		if f.pythonStyle {
			for len(v) > 0 && isSpaceC(v[0]) {
				v = v[1:]
			}
		}
		last := &f.entries[len(f.entries)-1]
		joined := valueOf(last.value) + "\n" + v
		last.value = &joined
		last.lineNumber = line

		effectiveCommentAfter := commentAfter
		if last.commentAfterValue != nil && commentAfter == nil {
			empty := ""
			effectiveCommentAfter = &empty
		}
		if effectiveCommentAfter != nil {
			if last.commentAfterValue != nil {
				joined := *last.commentAfterValue + "\n" + *effectiveCommentAfter
				last.commentAfterValue = &joined
			} else {
				s := "\n" + *effectiveCommentAfter
				last.commentAfterValue = &s
			}
		}
		return nil
	}

	grp := groupOrNone(group)
	f.internGroup(grp)
	e := entry{
		group:      grp,
		key:        strings.TrimRight(key, " \t\n\v\f\r"),
		lineNumber: line,
		quotes:     quotes,
	}
	if value != nil {
		v := *value
		e.value = &v
	}
	if commentBefore != nil {
		s := *commentBefore
		e.commentBeforeKey = &s
	}
	if commentAfter != nil {
		s := *commentAfter
		e.commentAfterValue = &s
	}
	f.entries = append(f.entries, e)
	return nil
}

// joinSameEntries mirrors getfilecontents.c's join_same_entries, run after
// parsing when the JOIN_SAME_ENTRIES option is set: entries sharing a
// group+key are merged into the first occurrence.
func joinSameEntries(f *File) {
	for i := 0; i < len(f.entries); i++ {
		for j := i + 1; j < len(f.entries); j++ {
			if f.entries[i].group != f.entries[j].group || f.entries[i].key != f.entries[j].key {
				continue
			}

			jv := f.entries[j].value
			if jv == nil || *jv == "" {
				empty := ""
				f.entries[i].value = &empty
			} else {
				post := *jv
				for len(post) > 0 && isSpaceC(post[0]) {
					post = post[1:]
				}
				joined := valueOf(f.entries[i].value) + "\n" + post
				f.entries[i].value = &joined
			}

			if f.entries[j].commentBeforeKey != nil && *f.entries[j].commentBeforeKey != "" {
				joined := valueOf(f.entries[i].commentBeforeKey) + "\n" + *f.entries[j].commentBeforeKey
				f.entries[i].commentBeforeKey = &joined
			}

			if jv == nil || *jv == "" {
				f.entries[i].commentAfterValue = nil
			} else if f.entries[j].commentAfterValue != nil && *f.entries[j].commentAfterValue != "" {
				post := *f.entries[j].commentAfterValue
				for len(post) > 0 && isSpaceC(post[0]) {
					post = post[1:]
				}
				if f.entries[i].commentAfterValue == nil {
					f.entries[i].commentAfterValue = &post
				} else {
					joined := *f.entries[i].commentAfterValue + "\n" + post
					f.entries[i].commentAfterValue = &joined
				}
			}
		}
	}
}
