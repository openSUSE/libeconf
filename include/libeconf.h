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

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

// Generic macro calls setter function depending on value type
// Use: setValue(Key_File *key_file, char *group, char *key, _generic_ value);
// Replace _generic_ with one of the supported value types.
// Supported Types: int, long, unsigned int, unsigned long, float, double,
// string (as *char).
// Note: Does not detect "yes", "no", 1 and 0 as boolean type. If you want to
// set a bool value use "true" or "false" or use setBoolValue() directly.
#define econf_setValue(kf, group, key, value) (( \
  _Generic((value), \
    int: econf_setIntValue, long: econf_setIntValue, \
    unsigned int: econf_setUIntValue, unsigned long: econf_setUIntValue, \
    float: econf_setFloatValue, \
    double: econf_setDoubleValue, \
    char*: econf_setStringValue)) \
(kf, group, key, value))

typedef struct Key_File Key_File;

Key_File *econf_newKeyFile(char delimiter, char comment);
Key_File *econf_newIniFile(void);

// Process the file of the given file_name and save its contents into key_file
Key_File *econf_get_key_file(const char *file_name, char *delim,
                      const char comment);

// Merge the contents of two key files
Key_File *econf_merge_key_files(Key_File *usr_file, Key_File *etc_file);

// Write content of a Key_File struct to specified location
void econf_write_key_file(Key_File *key_file, const char *save_to_dir,
                    const char *file_name);

// Wrapper function to perform the merge in one step
void econf_merge_files(const char *save_to_dir, const char *file_name,
                 const char *etc_path, const char *usr_path,
                 char *delimiter, const char comment);

/* --- GETTERS --- */

char **econf_getGroups(Key_File *kf, size_t *length);
char **econf_getKeys(Key_File *kf, const char *group, size_t *length);
int32_t econf_getIntValue(Key_File *kf, char *group, char *key);
int64_t econf_getInt64Value(Key_File *kf, char *group, char *key);
uint32_t econf_getUIntValue(Key_File *kf, char *group, char *key);
uint64_t econf_getUInt64Value(Key_File *kf, char *group, char *key);
float econf_getFloatValue(Key_File *kf, char *group, char *key);
double econf_getDoubleValue(Key_File *kf, char *group, char *key);
char *econf_getStringValue(Key_File *kf, char *group, char *key);
bool econf_getBoolValue(Key_File *kf, char *group, char *key);

/* --- SETTERS --- */

void econf_setIntValue(Key_File *kf, char *group, char *key, int64_t value);
void econf_setUIntValue(Key_File *kf, char *group, char *key, uint64_t value);
void econf_setFloatValue(Key_File *kf, char *group, char *key, float value);
void econf_setDoubleValue(Key_File *kf, char *group, char *key, double value);
void econf_setStringValue(Key_File *kf, char *group, char *key, char *value);
void econf_setBoolValue(Key_File *kf, char *group, char *key, char *value);

/* --- HELPERS --- */

// Free an array of type char** created by econf_getGroups() or econf_getKeys()
void econf_afree(char **array);

// Free memory allocated by key_file
void econf_destroy(Key_File *key_file);

