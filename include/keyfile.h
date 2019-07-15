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

#include <stdlib.h>

typedef struct Key_File {
  struct file_entry {
    char *group, *key, *value;
  } * file_entry;
  size_t length;
  char delimiter, comment;
} Key_File;
