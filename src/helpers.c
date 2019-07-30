/*
  Author: Pascal Arlt <parlt@suse.com>
  Copyright (C) 2019 SUSE Linux GmbH

  Licensed under the GNU Lesser General Public License Version 2.1

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, see
  <http://www.gnu.org/licenses/>.
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

  buffer[(*vlen)++] = 0;
  return buffer;
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

// Free memory allocated by key_file
void destroy(Key_File key_file) {
  for (int i = 0; i < key_file.alloc_length; i++) {
    free(key_file.file_entry[i].group);
    free(key_file.file_entry[i].key);
    free(key_file.file_entry[i].value);
  }
  free(key_file.file_entry);
}

// Wrapper function to free memory of merged file
void destroy_merged_file(Key_File merged_file) { free(merged_file.file_entry); }
