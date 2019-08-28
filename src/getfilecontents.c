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
#include "../include/getfilecontents.h"
#include "../include/helpers.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Fill the Key File struct with values from the given file handle
econf_err
fill_key_file(Key_File *read_file, FILE *kf, const char *delim) {
  // KEY_FILE_DEFAULT_LENGTH: Default number of key-value pairs to be
  // allocated in key_file
  // LLEN: Base number of chars in a key, value or group name
  // Once the value of these is exceeded memory space of double the value
  // is allocated
  const size_t LLEN = 8;
  size_t file_length = 0, lnum = KEY_FILE_DEFAULT_LENGTH, llen = LLEN, vlen = 0;
  char ch;

  // Allocate memory for the Key_File based on KEY_FILE_DEFAULT_LENGTH
  struct file_entry *fe = malloc(KEY_FILE_DEFAULT_LENGTH * sizeof(struct file_entry));
  if (fe == NULL)
    return ECONF_NOMEM;

  char *buffer = malloc(LLEN);
  if (buffer == NULL)
    {
      free (fe);
      return ECONF_NOMEM;
    }

  fe->group = strdup(KEY_FILE_NULL_VALUE);
  if (fe->group == NULL)
    {
      free (buffer);
      free (fe);
      return ECONF_NOMEM;
    }
  fe->key = KEY_FILE_NULL_VALUE;

  while ((ch = getc(kf)) != EOF) {
    if (vlen >= llen) {
      char *tmp = realloc(buffer, llen * 2);
      if (!tmp) {
        free (buffer);
        free (fe->group);
        free (fe);
        return ECONF_NOMEM;
      }
      buffer = tmp;
      llen *= 2;
    }
    if (ch == '\n') {
      if (vlen == 0 && !strcmp(fe[file_length].key, KEY_FILE_NULL_VALUE))
	continue;
      end_of_line(&fe, &file_length, &lnum, vlen, buffer);
    }
    // If the current char is the delimiter consider the part before to
    // be a key.
    else if (strchr(delim, ch) != NULL &&
             !strcmp(fe[file_length].key, KEY_FILE_NULL_VALUE)) {
      if(!file_length) { read_file->delimiter = ch; }
      buffer = clearblank(&vlen, buffer);
      fe[file_length].key = strndup(buffer, vlen);
      if (fe[file_length].key == NULL)
	return ECONF_NOMEM; /* TODO: try to cleanup memory */
    }
    // If the line contains the given comment char ignore the rest
    // of the line and proceed with the next
    else if (ch == read_file->comment) {
      if (vlen != 0) { end_of_line(&fe, &file_length, &lnum, vlen, buffer); }
      vlen = getline(&buffer, &llen, kf);
    }
    // Default case: append the char to the buffer
    else {
      buffer[vlen++] = ch;
      continue;
    }
    vlen = 0;
  }
  free(buffer);
  // Check if the file is really at its end after EOF is encountered.
  if (!feof(kf)) {
    read_file->length = -EBADF;
    return ECONF_ERROR;
  }
  if (!strcmp(fe->key, KEY_FILE_NULL_VALUE)) {
    fe->key = NULL;
    free(fe->group);
  }
  read_file->length = file_length;
  read_file->alloc_length = file_length;
  struct file_entry *tmp = realloc(fe, file_length * sizeof(struct file_entry));
  if (!tmp) {
    free(fe);
    return ECONF_NOMEM;
  }
  read_file->file_entry = tmp;

  return ECONF_SUCCESS;
}

// Write the group/value entry to the given file_entry
void end_of_line(struct file_entry **fe, size_t *len, size_t *lnum, size_t vlen,
                 char *buffer) {
  // Remove potential whitespaces from beginning and end
  buffer = clearblank(&vlen, buffer);
  // If a newline char is encountered and the line had no delimiter
  // the line is expected to be a group
  // In this case key is not set
  if (!strcmp((*fe)[*len].key, KEY_FILE_NULL_VALUE)) {
    if(!*len) { free((*fe)->group); }
    (*fe)[*len].group = strndup(buffer, vlen); /* XXX ENOMEM check missing */
  } else {
    // If the line is no new group copy the group from the previous line
    if (*len && !strcmp((*fe)[*len].group, KEY_FILE_NULL_VALUE)) {
      (*fe)[*len].group = strdup((*fe)[*len - 1].group); /* XXX ENOMEM check missing */
    }
    // If the line had a delimiter everything after the delimiter is
    // considered to be a value
    (*fe)[*len].value = strndup(buffer, vlen); /* XXX ENOMEM check missing */
    // Perform memory check and increase len by one
    new_kf_line(fe, len, lnum);
  }
}

// Check whether the key file has enough memory allocated, if not realloc
void new_kf_line(struct file_entry **fe, size_t *file_length, size_t *lnum) {
  if (++(*file_length) >= *lnum) {
    *fe = realloc(*fe, *lnum * 2 * sizeof(struct file_entry)); /* XXX ENOMEM check missing */
    (*lnum) *= 2;
  }
  (*fe)[*file_length].group = KEY_FILE_NULL_VALUE;
  (*fe)[*file_length].key = KEY_FILE_NULL_VALUE;
}
