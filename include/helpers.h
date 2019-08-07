/*
  Copyright (C) 2019 SUSE LLC
  Author: Pascal Arlt <parlt@suse.com>

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#pragma once

#include "keyfile.h"

// Combine file path and file name
char *combine_strings(const char *string_one, const char *string_two,
                      const char delimiter);

// Remove whitespace from beginning and end, append string terminator
char *clearblank(size_t *vlen, char *string);

// Returns the absolute path of a given path
char *get_absolute_path(const char *path);

// Remove '[' and ']' from beginning and end
char *stripbrackets(char *string);

// Add '[' and ']' to the given string
char *addbrackets(char *string);

// Set default value defined in include/defines.h
void initialize(Key_File *key_file, size_t num);

// Return the lower case version of a string
char *toLowerCase(char *str);

// Turn given string into a hash value
size_t hashstring(char *str);

// Look for matching key
size_t find_key(Key_File key_file, char *group, char *key);

// Append a new key to an existing Key_File
void new_key(Key_File *key_file, char *group, char *key);

// Set value for the given group, key combination. If the combination
// does not exist it is created.
void setKeyValue(void (*function) (Key_File*, size_t, void*),
                 Key_File *kf, char *group, char *key, void *value);
