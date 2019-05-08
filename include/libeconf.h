/*
  Author: Pascal Arlt <parlt@suse.com>
  Copyright (C) 2019 SUSE Linux GmbH

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KEY_MERGER_H
#define KEY_MERGER_H

#include <errno.h>
#include <stddef.h>

extern int errno;

typedef struct {
  char *key, *value, *group;
} Key_File;

// Process the file of the given file_name and save its contents into key_file
void get_key_file(Key_File **key_file, size_t *length, const char *file_name, const char delim, const char comment);

// Merge the contents of two key files
void merge_key_files(Key_File **merge_file, size_t *merge_length, Key_File *usr_file, size_t usr_length, Key_File *etc_file, size_t etc_length);

// Write content of a Key_File struct to specified location
void write_key_file(Key_File *key_file, size_t merge_length, const char *save_to_dir, const char *file_name, const char delimiter);

// Wrapper function to perform the merge in one step
void merge_files(const char *save_to_dir, const char *file_name, const char *etc_path, const char *usr_path, const char delimiter, const char comment);

/* --- HELPERS --- */

// Combine file path and file name
char* combine_path_name(const char *file_path, const char *file_name);

// Free memory allocated by key_file
void destroy(Key_File *key_file, size_t length);

#endif
