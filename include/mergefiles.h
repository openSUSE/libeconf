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

#include <stddef.h>

// Insert the content of "etc_file.file_entry" into "fe" if the group equals
// "[]", ef: etc_file
size_t insert_nogroup(struct file_entry **fe, Key_File *ef);

// Merge contents from existing usr_file groups
// uf: usr_file, ef: etc_file
size_t merge_existing_groups(struct file_entry **fe, Key_File *uf, Key_File *ef,
                             const size_t etc_start);

// Add entries from etc_file exclusive groups
size_t add_new_groups(struct file_entry **fe, Key_File *uf, Key_File *ef,
                      const size_t merge_length);
