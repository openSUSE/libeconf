/*
  Copyright (C) 2019, 2020 SUSE LLC
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
#include "defines.h"
#include "helpers.h"

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
void initialize(econf_file *key_file, size_t num) {
  key_file->file_entry[num].group = setGroupList(key_file, KEY_FILE_NULL_VALUE);
  key_file->file_entry[num].key = strdup(KEY_FILE_NULL_VALUE);
  key_file->file_entry[num].value = strdup(KEY_FILE_NULL_VALUE);
  key_file->file_entry[num].comment_before_key = NULL;
  key_file->file_entry[num].comment_after_value = NULL;
  key_file->file_entry[num].quotes = false;
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
  if (!string)
    return NULL;
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
  if (string == NULL)
    return NULL;
  size_t length = strlen(string);
  if (!(*string == '[' && string[length - 1] == ']')) {
    char *buffer = malloc(length + 3);
    if (buffer == NULL)
      return NULL;
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
econf_err find_key(econf_file key_file, const char *group, const char *key, size_t *num) {
	char *grp = (!group || !*group) ? strdup(KEY_FILE_NULL_VALUE) : strdup(group);
  if (grp == NULL)
    return  ECONF_NOMEM;
  if (!key || !*key) {
    free(grp);
    return ECONF_ERROR;
  }
  for (size_t i = 0; i < key_file.length; i++) {
    if (!strcmp(key_file.file_entry[i].group, grp) &&
        !strcmp(key_file.file_entry[i].key, key)) {
      free(grp);
      *num = i;
      return ECONF_SUCCESS;
    }
  }
  // Key not found
  free(grp);
  return ECONF_NOKEY;
}

// Append a new key to an existing econf_file
// TODO: If the group is already known the new key should be appended
// at the end of that group not at the end of the file.
static econf_err
new_key (econf_file *key_file, const char *group, const char *key) {
  econf_err error;
  char *grp = (!group || !*group) ? strdup(KEY_FILE_NULL_VALUE) : strdup(group);
  if (grp == NULL)
    return ECONF_NOMEM;
  if (key_file == NULL || key == NULL)
    {
      free(grp);
      return ECONF_ERROR;
    }
  if ((error = key_file_append(key_file))) {
    free(grp);
    return error;
  }
  if ((error = setGroup(key_file, key_file->length - 1, grp))) {
    free(grp);
    return error;
  }
  free(grp);
  return setKey(key_file, key_file->length - 1, key);
}

// Set value for the given group, key combination. If the combination
// does not exist it is created.
// TODO: function/void pointer might not be necessary if the value is converted
// into a string beforehand.
econf_err setKeyValue(econf_err (*function) (econf_file*, size_t, const void*),
		      econf_file *kf, const char *group, const char *key,
		      const void *value)
{
  size_t num;
  econf_err error = find_key(*kf, group, key, &num);
  if (error) {
    if (error != ECONF_NOKEY) {
      return error;
    }
    if ((error = new_key(kf, group, key))) {
      return error;
    }
    num = kf->length - 1;
  }
  return function(kf, num, value);
}

struct file_entry cpy_file_entry(econf_file *dest_kf, struct file_entry fe) {
  struct file_entry copied_fe;
  copied_fe.group = setGroupList(dest_kf, fe.group);
  copied_fe.key = strdup(fe.key);
  if (fe.value)
    copied_fe.value = strdup(fe.value);
  else
    copied_fe.value = NULL;
  if (fe.comment_before_key)
    copied_fe.comment_before_key = strdup(fe.comment_before_key);
  else
    copied_fe.comment_before_key = NULL;
  if (fe.comment_after_value)
    copied_fe.comment_after_value = strdup(fe.comment_after_value);
  else
    copied_fe.comment_after_value = NULL;  
  copied_fe.line_number = fe.line_number;
  copied_fe.quotes = false;
  return copied_fe;
}

/* Handle groups in an string array */
char *getFromGroupList(econf_file *key_file, const char *name) {
  char *ret = NULL;
  for (int i = 0; i < key_file->group_count; i++) {
    if (!strcmp(key_file->groups[i], name)) {
      ret = key_file->groups[i];
      i = key_file->group_count;
    }
  }
  return ret;
}

char *setGroupList(econf_file *key_file, const char *name) {
  char *ret = getFromGroupList(key_file, name);
  if (ret != NULL)
    return ret;
  key_file->group_count++;
  key_file->groups =
    realloc(key_file->groups, (key_file->group_count +1) * sizeof(char *));
  if (key_file->groups == NULL) {
    key_file->group_count--;
  } else {
    key_file->groups[key_file->group_count] = NULL;
    key_file->groups[key_file->group_count-1] = strdup(name);
    ret = key_file->groups[key_file->group_count-1];
  }
  return ret;
}
