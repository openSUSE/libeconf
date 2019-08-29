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

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct econf_file {
  struct file_entry {
    char *group, *key, *value;
  } * file_entry;
  size_t length, alloc_length;
  char delimiter, comment;
  bool on_merge_delete;
} econf_file;

econf_err key_file_append(econf_file *key_file);

/* GETTERS */
econf_err getIntValueNum(econf_file key_file, size_t num, int32_t *result);
econf_err getInt64ValueNum(econf_file key_file, size_t num, int64_t *result);
econf_err getUIntValueNum(econf_file key_file, size_t num, uint32_t *result);
econf_err getUInt64ValueNum(econf_file key_file, size_t num, uint64_t *result);
econf_err getFloatValueNum(econf_file key_file, size_t num, float *result);
econf_err getDoubleValueNum(econf_file key_file, size_t num, double *result);
econf_err getStringValueNum(econf_file key_file, size_t num, char **result);
econf_err getBoolValueNum(econf_file key_file, size_t num, bool *result);

/* SETTERS */
econf_err setGroup(econf_file *key_file, size_t num, const char *value);
econf_err setKey(econf_file *key_file, size_t num, const char *value);
econf_err setIntValueNum(econf_file *key_file, size_t num, const void *value);
econf_err setInt64ValueNum(econf_file *key_file, size_t num, const void *value);
econf_err setUIntValueNum(econf_file *key_file, size_t num, const void *value);
econf_err setUInt64ValueNum(econf_file *key_file, size_t num, const void *value);
econf_err setFloatValueNum(econf_file *key_file, size_t num, const void *value);
econf_err setDoubleValueNum(econf_file *key_file, size_t num, const void *value);
econf_err setStringValueNum(econf_file *key_file, size_t num, const void *value);
econf_err setBoolValueNum(econf_file *key_file, size_t num, const void *value);
