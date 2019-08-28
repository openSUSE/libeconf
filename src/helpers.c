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

#include "libeconf.h"
#include "../include/defines.h"
#include "../include/helpers.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

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
  if (!*vlen) return string;

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

char *get_absolute_path(const char *path, econf_err *error) {
  char *absolute_path;
  if(*path != '/') {
    char buffer[PATH_MAX];
    if(!realpath(path, buffer)) {
      if (error)
	*error = ECONF_NOFILE;
      return NULL;
    }
    absolute_path = strdup(buffer);
  } else {
    absolute_path = strdup(path);
  }
  if (absolute_path == NULL && error)
    *error = ECONF_NOMEM;

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
char *addbrackets(const char *string) {
  size_t length = strlen(string);
  if (!(*string == '[' && string[length - 1] == ']')) {
    char *buffer = malloc(length + 3);
    char *cp = buffer;
    *cp++ = '[';
    cp = stpcpy (cp, string);
    *cp++ = ']';
    *cp = '\0';
    return buffer;
  }
  return strdup(string);
}

// Turn the given string into lower case
char *toLowerCase(char *string) {
  char *ptr = string;
  while (*string)
    {
      *string = tolower(*string);
      string++;
    }
  return ptr;
}

// Turn the given string into a hash value
// Hash function djb2 from Dan J. Bernstein
size_t hashstring(const char *string) {
  size_t hash = 5381;
  char c;
  while ((c = *string++)) { hash = ((hash << 5) + hash) + c; }
  return hash;
}

// Look for matching key
size_t find_key(Key_File key_file, const char *group, const char *key, econf_err *error) {
  char *grp = (!group || !*group) ? strdup(KEY_FILE_NULL_VALUE) :
               addbrackets(group);
  if (grp == NULL) {
    if (error) *error = ECONF_NOMEM;
    return -1;
  }
  if (!key || !*key) {
    if (error) *error = ECONF_ERROR;
    free(grp);
    return -1;
  }
  for (size_t i = 0; i < key_file.length; i++) {
    if (!strcmp(key_file.file_entry[i].group, grp) &&
        !strcmp(key_file.file_entry[i].key, key)) {
      free(grp);
      return i;
    }
  }
  // Key not found
  if (error)
    *error = ECONF_NOKEY;
  free(grp);
  return -1;
}

// Append a new key to an existing Key_File
// TODO: If the group is already known the new key should be appended
// at the end of that group not at the end of the file.
static bool
new_key (Key_File *key_file, const char *group, const char *key, econf_err *error) {
  char *grp = (!group || !*group) ? strdup(KEY_FILE_NULL_VALUE) :
               addbrackets(group);
  if (grp == NULL) {
    if (error) *error = ECONF_NOMEM;
    return false;
  }
  if (key_file == NULL || key == NULL)
    {
      if (error) *error = ECONF_ERROR;
      free(grp);
      return false;
    }
  if (!key_file_append(key_file, error)) {
    free(grp);
    return false;
  }
  if (!setGroup(key_file, key_file->length - 1, grp, error)) {
    free(grp);
    return false;
  }
  free(grp);
  return setKey(key_file, key_file->length - 1, key, error);
}

// Set value for the given group, key combination. If the combination
// does not exist it is created.
// TODO: function/void pointer might not be necessary if the value is converted
// into a string beforehand.
bool setKeyValue(bool (*function) (Key_File*, size_t, const void*, econf_err *),
		 Key_File *kf, const char *group, const char *key,
		 const void *value, econf_err *error) {
  econf_err local_err = ECONF_SUCCESS;
  int num = find_key(*kf, group, key, &local_err);
  if (num == -1) {
    if (local_err && local_err != ECONF_NOKEY) {
      if (error) *error = local_err;
	    return false;
	  }
    if (!new_key(kf, group, key, error)) {
      return false;
    }
    num = kf->length - 1;
  }
  return function(kf, num, value, error);
}

struct file_entry cpy_file_entry(struct file_entry fe) {
  struct file_entry copied_fe;
  copied_fe.group = strdup(fe.group);
  copied_fe.key = strdup(fe.key);
  copied_fe.value = strdup(fe.value);
  return copied_fe;
}
