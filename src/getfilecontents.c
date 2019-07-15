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
#include "../include/getfilecontents.h"
#include "../include/helpers.h"

#include <errno.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Fill the Key File struct with values from the given file handle
Key_File fill_key_file(Key_File read_file, FILE *kf, const char *delim) {
  // KEY_FILE_DEFAULT_LENGTH: Default number of key-value pairs to be
  // allocated in key_file
  // LLEN: Base number of chars in a key, value or group name
  // Once the value of these is exceeded memory space of double the value
  // is allocated
  const size_t LLEN = 8;
  size_t file_length = 0, lnum = KEY_FILE_DEFAULT_LENGTH, llen = LLEN, vlen = 0;
  char ch;

  regex_t regex;
  regcomp(&regex, delim, 0);

  // Allocate memory for the Key_File based on KEY_FILE_DEFAULT_LENGTH
  struct file_entry *fe = malloc(KEY_FILE_DEFAULT_LENGTH * sizeof(struct file_entry));
  fe->group = malloc(3), fe->key = NULL;
  snprintf(fe->group, 3, "[]");
  char *buffer = malloc(LLEN);

  while ((ch = getc(kf)) != EOF) {
    if (vlen >= llen) {
      buffer = realloc(buffer, llen * 2);
      llen *= 2;
    }
    if (ch == '\n') {
      if (vlen == 0 && !fe[file_length].key)
        continue;
      end_of_line(&fe, &file_length, &lnum, vlen, buffer);
    }
    // If the current char is the delimiter consider the part before to
    // be a key.
    else if (!regexec(&regex, &ch, 0, NULL, 0) && !fe[file_length].key) {
      if(!file_length)
        read_file.delimiter = ch;
      buffer = clearblank(&vlen, buffer);
      fe[file_length].key = malloc(vlen);
      snprintf(fe[file_length].key, vlen, buffer);
    }
    // If the line contains the given comment char ignore the rest
    // of the line and proceed with the next
    else if (ch == read_file.comment) {
      if (vlen != 0)
        end_of_line(&fe, &file_length, &lnum, vlen, buffer);
      getline(&buffer, &llen, kf);
    }
    // Default case: append the char to the buffer
    else {
      buffer[vlen++] = ch;
      continue;
    }
    vlen = 0;
  }
  free(buffer);
  regfree(&regex);
  // Check if the file is really at its end after EOF is encountered.
  if (!feof(kf)) {
    read_file.length = -EBADF;
    return read_file;
  }
  read_file.length = file_length;
  read_file.alloc_length = file_length;
  fe = realloc(fe, file_length * sizeof(struct file_entry));
  read_file.file_entry = fe;
  return read_file;
}

// Write the group/value entry to the given file_entry
void end_of_line(struct file_entry **fe, size_t *len, size_t *lnum, size_t vlen,
                 char *buffer) {
  // Remove potential whitespaces from beginning and end
  buffer = clearblank(&vlen, buffer);
  // If a newline char is encountered and the line had no delimiter
  // the line is expected to be a group
  // In this case key is not set
  if (!(*fe)[*len].key) {
    if (!*len)
      free((*fe)->group);
    (*fe)[*len].group = malloc(vlen);
    snprintf((*fe)[*len].group, vlen, buffer);
  } else {
    // If the line is no new group copy the group from the previous line
    if (*len && !strcmp((*fe)[*len].group, "[]")) {
      size_t tmp = strlen((*fe)[*len - 1].group) + 1;
      (*fe)[*len].group = malloc(tmp);
      snprintf((*fe)[*len].group, tmp, (*fe)[*len - 1].group);
    }
    // If the line had a delimiter everything after the delimiter is
    // considered to be a value
    (*fe)[*len].value = malloc(vlen);
    snprintf((*fe)[*len].value, vlen, buffer);
    // Perform memory check and increase len by one
    new_kf_line(fe, len, lnum);
  }
}

// Check whether the key file has enough memory allocated, if not realloc
void new_kf_line(struct file_entry **fe, size_t *file_length, size_t *lnum) {
  if (++(*file_length) >= *lnum) {
    *fe = realloc(*fe, *lnum * 2 * sizeof(struct file_entry));
    (*lnum) *= 2;
  }
  (*fe)[*file_length].group = "[]";
  (*fe)[*file_length].key = NULL;
}
