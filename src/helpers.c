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

#include "../include/defines.h"
#include "../include/helpers.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Combine file path and file name
char *combine_strings(const char *string_one, const char *string_two,
                      const char delimiter) {
  size_t combined_len = strlen(string_one) + strlen(string_two) + 2;
  char *combined = malloc(combined_len);
  snprintf(combined, combined_len, "%s%c%s", string_one, delimiter, string_two);
  return combined;
}

// Set null value defined in include/defines.h
void initialize(Key_File *key_file, size_t num) {
  key_file->file_entry[num].group = strdup(KEY_FILE_NULL_VALUE);
  key_file->file_entry[num].key = strdup(KEY_FILE_NULL_VALUE);
  key_file->file_entry[num].value = strdup(KEY_FILE_NULL_VALUE);
}

// Remove whitespace from beginning and end, append string terminator
char *clearblank(size_t *vlen, char *string) {
  char *buffer = string, *ptr = string;
  string[*vlen] = 0;

  while (*string != 0) {
    if (ptr == buffer && (*string == ' ' || *string == '\t')) {
      (*vlen)--;
    } else {
      *ptr++ = *string;
    }
    string++;
  }
  while (buffer[*vlen - 1] == ' ' || buffer[*vlen - 1] == '\t')
    (*vlen)--;

  buffer[*vlen] = 0;
  return buffer;
}

char *get_absolute_path(const char *path) {
  char *absolute_path;
  if(*path != '/') {
    char buffer[256];
    if(!realpath(path, buffer)) {
      errno = ENOENT;
      return NULL;
    }
    absolute_path = strdup(buffer);
  } else {
    absolute_path = strdup(path);
  }
  return absolute_path;
}

// Remove '[' and ']' from beginning and end
char *stripbrackets(char *string) {
  char *ptr = string, *buffer = string;
  size_t length = strlen(string) - 1;
  if (*string == '[' && string[length] == ']') {
    while (*(++string) != ']') { *buffer++ = *string; }
    *buffer = 0;
  }
  return ptr;
}

// Add '[' and ']' to the given string
char *addbrackets(char *string) {
  size_t length = strlen(string);
  if (!(*string == '[' && string[length - 1] == ']')) {
    char *buffer = malloc(length + 3);
    sprintf(buffer, "[%s]", string);
    free(string);
    return buffer;
  }
  return string;
}

// Turn the given string into lower case
char *toLowerCase(char *string) {
  char *ptr = string;
  while (*string)
    *string++ = tolower(*string);
  return ptr;
}

// Turn the given string into a hash value
// Hash function djb2 from Dan J. Bernstein
size_t hashstring(char *string) {
  size_t hash = 5381;
  char c;
  while ((c = *string++)) { hash = ((hash << 5) + hash) + c; }
  return hash;
}

// Look for matching key
size_t find_key(Key_File key_file, char *group, char *key) {
  size_t g = hashstring(group), k = hashstring(key);
  for (int i = 0; i < key_file.length; i++) {
    if (g == hashstring(key_file.file_entry[i].group) &&
        k == hashstring(key_file.file_entry[i].key)) {
      return i;
    }
  }
  // Key not found
  return -1;
}

// Append a new key to an existing Key_File
void new_key(Key_File *key_file, char *group, char *key) {
  key_file_append(key_file);
  setGroup(key_file, key_file->length - 1, group);
  setKey(key_file, key_file->length - 1, key);
}

// Set value for the given group, key combination. If the combination
// does not exist it is created.
// TODO: function/void pointer might not be necessary if the value is converted
// into a string beforehand.
void setKeyValue(void (*function) (Key_File*, size_t, void*), Key_File *kf, char *group, char *key, void *value) {
  char *tmp = strdup(group);
  tmp = addbrackets(tmp);
  int num = find_key(*kf, tmp, key);
  if (num != -1) {
    free(tmp);
    function(kf, num, value);
  } else {
    new_key(kf, tmp, key);
    function(kf, kf->length - 1, value);
  }
}
