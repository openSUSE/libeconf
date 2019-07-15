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

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct Key_File {
  struct file_entry {
    char *group, *key, *value;
  } * file_entry;
  size_t length, alloc_length;
  char delimiter, comment;
} Key_File;

Key_File newKeyFile(char delimiter, char comment);
Key_File newIniFile();

/* GETTERS */
int32_t getIntValueNum(Key_File key_file, size_t num);
uint32_t getUIntValueNum(Key_File key_file, size_t num);
char *getStringValueNum(Key_File key_file, size_t num);
bool getBoolValueNum(Key_File key_file, size_t num);

/* SETTERS */
void setGroup(Key_File *key_file, size_t num, char *value);
void setKey(Key_File *key_file, size_t num, char *value);
void setIntValueNum(Key_File *key_file, size_t num, void *value);
void setUIntValueNum(Key_File *key_file, size_t num, void *value);
void setStringValueNum(Key_File *key_file, size_t num, void *value);
void setBoolValueNum(Key_File *key_file, size_t num, void *value);

