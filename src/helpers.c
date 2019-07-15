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

#include "../include/helpers.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Combine file path and file name
char *combine_path_name(const char *file_path, const char *file_name) {
  size_t combined_len = strlen(file_path) + strlen(file_name) + 2;
  char *combined = malloc(combined_len);
  snprintf(combined, combined_len, "%s/%s", file_path, file_name);
  return combined;
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

// Free memory allocated by key_file
void destroy(Key_File key_file) {
  for (int i = 0; i < key_file.length; i++) {
    free(key_file.file_entry[i].group);
    free(key_file.file_entry[i].key);
    free(key_file.file_entry[i].value);
  }
  free(key_file.file_entry);
}

// Wrapper function to free memory of merged file
void destroy_merged_file(Key_File merged_file) { free(merged_file.file_entry); }
