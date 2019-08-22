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

#include <stddef.h>

// Insert the content of "etc_file.file_entry" into "fe" if there is no
// group specified
// "[]", ef: etc_file
size_t insert_nogroup(struct file_entry **fe, Key_File *ef);

// Merge contents from existing usr_file groups
// uf: usr_file, ef: etc_file
size_t merge_existing_groups(struct file_entry **fe, Key_File *uf, Key_File *ef,
                             const size_t etc_start);

// Add entries from etc_file exclusive groups
size_t add_new_groups(struct file_entry **fe, Key_File *uf, Key_File *ef,
                      const size_t merge_length);

// Returns the default dirs to iterate through when merging
char **get_default_dirs(const char *usr_conf_dir, const char *etc_conf_dir);

// Receives a list of config directories to look for and calls 'check_conf_dir'
Key_File **traverse_conf_dirs(Key_File **key_files, char *conf_dirs,
                              size_t *size, char *path, char *config_suffix,
                              char *delim, char comment);

// Check if the given directory exists. If so look for config files
// with the given suffix
Key_File **check_conf_dir(Key_File **key_files, size_t *size, char *path,
                          char *config_suffix, char *delim, char comment);

// Merge an array of given Key_Files into one
Key_File *merge_Key_Files(Key_File **key_files);
