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
#include "../include/keyfile.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Create a new Key_File. Allocation is based on KEY_FILE_DEFAULT_LENGTH
// defined in include/defines.h
Key_File newKeyFile(char delimiter, char comment) {
  Key_File key_file;

  key_file.alloc_length = KEY_FILE_DEFAULT_LENGTH;
  key_file.length = 0;
  key_file.delimiter = delimiter;
  key_file.comment = comment;

  key_file.file_entry = malloc(KEY_FILE_DEFAULT_LENGTH * sizeof(struct file_entry));

  for (size_t i = 0; i < KEY_FILE_DEFAULT_LENGTH; i++) {
    set_default_value(&key_file, i);
  }

  return key_file;
}

Key_File newIniFile() {
  return newKeyFile('=', '#');
}
