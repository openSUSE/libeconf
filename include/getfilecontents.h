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

#include <stdio.h>

// Fill the Key File struct with values from the given file handle
void fill_key_file(Key_File *read_file, FILE *kf, const char *delim);

// Write the group/value entry to the given file_entry
void end_of_line(struct file_entry **fe, size_t *len, size_t *lnum, size_t vlen,
                 char *buffer);

// Check whether the key file has enough memory allocated, if not realloc
void new_kf_line(struct file_entry **fe, size_t *file_length, size_t *lnum);
