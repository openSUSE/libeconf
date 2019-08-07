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

#include "../include/mergefiles.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Insert the content of "etc_file.file_entry" into "fe" if there is no
// group specified
size_t insert_nogroup(struct file_entry **fe, Key_File *ef) {
  size_t etc_start = 0;
  while (etc_start < ef->length &&
         !strcmp(ef->file_entry[etc_start].group, KEY_FILE_NULL_VALUE)) {
    (*fe)[etc_start] = ef->file_entry[etc_start];
    etc_start++;
  }
  return etc_start;
}

// Merge contents from existing usr_file groups
// uf: usr_file, ef: etc_file
size_t merge_existing_groups(struct file_entry **fe, Key_File *uf, Key_File *ef,
                             const size_t etc_start) {
  char new_key;
  size_t merge_length = etc_start, tmp = etc_start, added_keys = etc_start;
  for (int i = 0; i <= uf->length; i++) {
    // Check if the group has changed in the last iteration
    if (i == uf->length ||
        (i && strcmp(uf->file_entry[i].group, uf->file_entry[i - 1].group))) {
      for (int j = etc_start; j < ef->length; j++) {
        // Check for matching groups
        if (!strcmp(uf->file_entry[i - 1].group, ef->file_entry[j].group)) {
          new_key = 1;
          for (int k = merge_length; k < i + tmp; k++) {
            // If an existing key is found in ef take the value from ef
            if (!strcmp((*fe)[k].key, ef->file_entry[j].key)) {
              (*fe)[k] = ef->file_entry[j];
              new_key = 0;
              break;
            }
          }
          // If a new key is found for an existing group append it to the group
          if (new_key)
            (*fe)[i + added_keys++] = ef->file_entry[j];
        }
      }
      merge_length = i + added_keys;
      // Temporary value to reduce amount of iterations in inner for loop
      tmp = added_keys;
    }
    if (i != uf->length)
      (*fe)[i + added_keys] = uf->file_entry[i];
  }
  return merge_length;
}

// Add entries from etc_file exclusive groups
size_t add_new_groups(struct file_entry **fe, Key_File *uf, Key_File *ef,
                      const size_t merge_length) {
  size_t added_keys = merge_length;
  char new_key;
  for (int i = 0; i < ef->length; i++) {
    if (!strcmp(ef->file_entry[i].group, KEY_FILE_NULL_VALUE))
      continue;
    new_key = 1;
    for (int j = 0; j < uf->length; j++) {
      if (!strcmp(uf->file_entry[j].group, ef->file_entry[i].group)) {
        new_key = 0;
        break;
      }
    }
    if (new_key)
      (*fe)[added_keys++] = ef->file_entry[i];
  }
  *fe = realloc(*fe, added_keys * sizeof(struct file_entry));
  return added_keys;
}
