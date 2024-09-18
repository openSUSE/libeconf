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

/* --- keyfile.h --- */

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

/* This file contains the definition of the econf_file struct declared in
   libeconf.h as well as the functions to get and set a specified element
   of the struct. All functions return an error code != 0 on error defined
   in libeconf.h.  */


/* Definition of the econf_file struct and its inner file_entry struct.  */
typedef struct econf_file {
  /* The file_entry struct contains the group, key and value of every
     key/value entry found in a config file or set via the set functions. If no
     group is found or provided the group is set to KEY_FILE_NULL_VALUE.  */
  struct file_entry {
    char *group, *key, *value;
    char *comment_before_key, *comment_after_value;
    uint64_t line_number;
    bool quotes; /*Value is enclosed by quotes*/
  } * file_entry;
  /* length represents the current amount of key/value entries in econf_file and
     alloc_length the the amount of currently allocated file_entry elements
     within the struct. If length would exceed alloc_length it's increased.  */
  size_t length, alloc_length;
  /* delimiter: char used to assign a value to a key
     comment: Used to specify which char to regard as comment indicator.
     These two variables will be used for writing the entries into a file only. */
  char delimiter, comment;
  /* Binary variable to determine whether econf_file should be freed after
     being merged with another econf_file.  */
  bool on_merge_delete;
  char *path;

  /* General options */

  /* Parsed entries with the same name will not be replaces but */
  /* will be joined to one entry. */
  bool join_same_entries;

  /* Using python text style. */
  /* e.b. Identations will be handled like multiline entries. */
  bool python_style;

  /* List of directories from which the configuration files have to be parsed. */
  /* The last entry has the highest priority. */
  char **parse_dirs;
  int parse_dirs_count;

  // configuration directories format
  char **conf_dirs;
  int conf_count;

  // groups
  char **groups;
  int group_count;

} econf_file;

/* Increases both length and alloc_length of key_file by one and initializes
   new elements of struct file_entry.  */
econf_err key_file_append(econf_file *key_file);

/* GETTERS */

/* Functions used to get a set value from key_file depending on num.
   Expects a pointer of fitting type and writes the result into the pointer.
   num corresponds to the respective instance of the file_entry array. */
econf_err getIntValueNum(econf_file key_file, size_t num, int32_t *result);
econf_err getInt64ValueNum(econf_file key_file, size_t num, int64_t *result);
econf_err getUIntValueNum(econf_file key_file, size_t num, uint32_t *result);
econf_err getUInt64ValueNum(econf_file key_file, size_t num, uint64_t *result);
econf_err getFloatValueNum(econf_file key_file, size_t num, float *result);
econf_err getDoubleValueNum(econf_file key_file, size_t num, double *result);
econf_err getStringValueNum(econf_file key_file, size_t num, char **result);
econf_err getBoolValueNum(econf_file key_file, size_t num, bool *result);
econf_err getCommentsNum(econf_file key_file, size_t num,
		      char **comment_before_key,
		      char **comment_after_value);
econf_err getLineNrNum(econf_file key_file, size_t num, uint64_t *line_nr);
econf_err getPath(econf_file key_file, char **path);

/* SETTERS */

/* Set the group of the file_entry element number num */
econf_err setGroup(econf_file *key_file, size_t num, const char *value);
/* Set the key of the file_entry element number num */
econf_err setKey(econf_file *key_file, size_t num, const char *value);

/* Functions used to set a value from key_file depending on num.
   Expects a void pointer to the value which is cast to the corresponding
   type inside the function. num corresponds to the respective instance of the
   file_entry array.  */
econf_err setIntValueNum(econf_file *key_file, size_t num, const void *value);
econf_err setInt64ValueNum(econf_file *key_file, size_t num, const void *value);
econf_err setUIntValueNum(econf_file *key_file, size_t num, const void *value);
econf_err setUInt64ValueNum(econf_file *key_file, size_t num, const void *value);
econf_err setFloatValueNum(econf_file *key_file, size_t num, const void *value);
econf_err setDoubleValueNum(econf_file *key_file, size_t num, const void *value);
econf_err setStringValueNum(econf_file *key_file, size_t num, const void *value);
econf_err setBoolValueNum(econf_file *key_file, size_t num, const void *value);

/* helper functions */

void print_key_file(const econf_file key_file);
