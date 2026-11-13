package econf

import (
	"strconv"
	"strings"
)

// intPrefix finds the longest prefix of s that strtol/strtoul(s, &endptr,
// 0) would consume: optional leading whitespace, optional sign, then a
// base-0 (0x hex / 0 octal / decimal) numeral. Unlike strconv.ParseInt, it
// does not require the whole string to be consumed - trailing garbage
// (e.g. "42abc") is simply ignored, matching strtol's endptr behavior.
func intPrefix(s string) (prefix string, ok bool) {
	i, n := 0, len(s)
	for i < n && isSpaceC(s[i]) {
		i++
	}
	start := i
	if i < n && (s[i] == '+' || s[i] == '-') {
		i++
	}
	digitsStart := i

	if i < n && s[i] == '0' {
		if i+1 < n && (s[i+1] == 'x' || s[i+1] == 'X') {
			hexStart := i + 2
			j := hexStart
			for j < n && isHexDigit(s[j]) {
				j++
			}
			if j > hexStart {
				i = j
			} else {
				// "0x" with no following hex digit: strtol only consumes
				// the leading "0".
				i = digitsStart + 1
			}
		} else {
			for i < n && s[i] >= '0' && s[i] <= '7' {
				i++
			}
		}
	} else {
		for i < n && s[i] >= '0' && s[i] <= '9' {
			i++
		}
	}

	if i == digitsStart {
		return "", false
	}
	return s[start:i], true
}

func isHexDigit(b byte) bool {
	return (b >= '0' && b <= '9') || (b >= 'a' && b <= 'f') || (b >= 'A' && b <= 'F')
}

func parseInt(s string, bitSize int) (int64, error) {
	prefix, ok := intPrefix(s)
	if !ok {
		return 0, ErrValueConversionError
	}
	n, err := strconv.ParseInt(prefix, 0, bitSize)
	if err != nil {
		return 0, ErrValueConversionError
	}
	return n, nil
}

// parseUint mirrors strtoul, which (like strtol) accepts a leading sign and
// interprets a negative value by wrapping it to the unsigned range.
func parseUint(s string, bitSize int) (uint64, error) {
	prefix, ok := intPrefix(s)
	if !ok {
		return 0, ErrValueConversionError
	}
	negative := strings.HasPrefix(prefix, "-")
	magnitude := prefix
	if negative || strings.HasPrefix(prefix, "+") {
		magnitude = prefix[1:]
	}
	n, err := strconv.ParseUint(magnitude, 0, bitSize)
	if err != nil {
		return 0, ErrValueConversionError
	}
	if negative {
		n = -n
		if bitSize < 64 {
			n &= (1 << uint(bitSize)) - 1
		}
	}
	return n, nil
}

// floatPrefix finds the longest prefix of s that strtof/strtod(s, &endptr)
// would consume: optional leading whitespace, optional sign, decimal
// digits with an optional fraction and exponent, or an inf/infinity/nan
// keyword. Hexadecimal floating-point literals (e.g. "0x1.8p3") are not
// supported; they are vanishingly rare in configuration files.
func floatPrefix(s string) (prefix string, ok bool) {
	i, n := 0, len(s)
	for i < n && isSpaceC(s[i]) {
		i++
	}
	start := i
	if i < n && (s[i] == '+' || s[i] == '-') {
		i++
	}

	rest := strings.ToLower(s[i:])
	switch {
	case strings.HasPrefix(rest, "infinity"):
		return s[start : i+8], true
	case strings.HasPrefix(rest, "inf"):
		return s[start : i+3], true
	case strings.HasPrefix(rest, "nan"):
		return s[start : i+3], true
	}

	digitsStart := i
	for i < n && isDigit(s[i]) {
		i++
	}
	hasIntDigits := i > digitsStart

	hasFracDigits := false
	if i < n && s[i] == '.' {
		i++
		fracStart := i
		for i < n && isDigit(s[i]) {
			i++
		}
		hasFracDigits = i > fracStart
	}

	if !hasIntDigits && !hasFracDigits {
		return "", false
	}

	if i < n && (s[i] == 'e' || s[i] == 'E') {
		save := i
		j := i + 1
		if j < n && (s[j] == '+' || s[j] == '-') {
			j++
		}
		expDigitsStart := j
		for j < n && isDigit(s[j]) {
			j++
		}
		if j > expDigitsStart {
			i = j
		} else {
			i = save
		}
	}

	return s[start:i], true
}

func isDigit(b byte) bool {
	return b >= '0' && b <= '9'
}

func parseFloat(s string, bitSize int) (float64, error) {
	prefix, ok := floatPrefix(s)
	if !ok {
		return 0, ErrValueConversionError
	}
	v, err := strconv.ParseFloat(prefix, bitSize)
	if err != nil {
		return 0, ErrValueConversionError
	}
	return v, nil
}
