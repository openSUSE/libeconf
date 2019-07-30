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

#pragma once

#include "keyfile.h"

// Combine file path and file name
char *combine_strings(const char *string_one, const char *string_two,
                      const char delimiter);

// Remove whitespace from beginning and end, append string terminator
char *clearblank(size_t *vlen, char *string);

// Remove '[' and ']' from beginning and end
char *stripbrackets(char *string);

// Add '[' and ']' to the given string
char *addbrackets(char *string);

// Set default value defined in include/defines.h
void initialize(Key_File *key_file, size_t num);

// Return the lower case version of a string
char *toLowerCase(char *str);

// Turn given string into a hash value
size_t hashstring(char *str);

// Look for matching key
size_t find_key(Key_File key_file, char *group, char *key);

// Append a new key to an existing Key_File
void new_key(Key_File *key_file, char *group, char *key);

// Set value for the given group, key combination. If the combination
// does not exist it is created.
void setKeyValue(void (*function) (Key_File*, size_t, void*),
                 Key_File *kf, char *group, char *key, void *value);

// Free memory allocated by key_file
void destroy(Key_File key_file);

// Wrapper function to free memory of merged file
void destroy_merged_file(Key_File merged_file);
