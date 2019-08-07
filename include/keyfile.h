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

typedef struct Key_File {
  struct file_entry {
    char *group, *key, *value;
  } * file_entry;
  size_t length, alloc_length;
  char delimiter, comment;
  bool on_merge_delete;
} Key_File;

void key_file_append(Key_File *key_file);

/* GETTERS */
int32_t getIntValueNum(Key_File key_file, size_t num);
int64_t getInt64ValueNum(Key_File key_file, size_t num);
uint32_t getUIntValueNum(Key_File key_file, size_t num);
uint64_t getUInt64ValueNum(Key_File key_file, size_t num);
float getFloatValueNum(Key_File key_file, size_t num);
double getDoubleValueNum(Key_File key_file, size_t num);
char *getStringValueNum(Key_File key_file, size_t num);
bool getBoolValueNum(Key_File key_file, size_t num);

/* SETTERS */
void setGroup(Key_File *key_file, size_t num, char *value);
void setKey(Key_File *key_file, size_t num, char *value);
void setIntValueNum(Key_File *key_file, size_t num, void *value);
void setUIntValueNum(Key_File *key_file, size_t num, void *value);
void setFloatValueNum(Key_File *key_file, size_t num, void *value);
void setDoubleValueNum(Key_File *key_file, size_t num, void *value);
void setStringValueNum(Key_File *key_file, size_t num, void *value);
void setBoolValueNum(Key_File *key_file, size_t num, void *value);
