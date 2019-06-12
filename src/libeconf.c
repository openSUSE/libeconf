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

#include "../include/libeconf.h"

#include "../include/getfiles.h"
#include "../include/helpers.h"
#include "../include/mergefiles.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Process the file of the given file_name and save its contents into key_file
Key_File get_key_file(const char *file_name, const char delim,
                      const char comment) {
  Key_File read_file = {.delimiter = delim, .comment = comment};

  // File handle for the given file_name
  FILE *kf = fopen(file_name, "rb");
  if (kf == NULL) {
    read_file.length = -ENOENT;
    return read_file;
  }

  read_file = fill_key_file(read_file, kf);

  fclose(kf);
  return read_file;
}

// Merge the contents of two key files
Key_File merge_key_files(Key_File *usr_file, Key_File *etc_file) {
  Key_File merge_file = {.delimiter = usr_file->delimiter,
                         .comment = usr_file->comment};
  struct file_entry *fe =
      malloc((etc_file->length + etc_file->length) * sizeof(struct file_entry));

  size_t merge_length = 0;

  if (!strcmp(etc_file->file_entry->group, "[]") &&
      strcmp(usr_file->file_entry->group, "[]"))
    merge_length = insert_nogroup(&fe, etc_file);
  merge_length = merge_existing_groups(&fe, usr_file, etc_file, merge_length);
  merge_file.length = add_new_groups(&fe, usr_file, etc_file, merge_length);

  merge_file.file_entry = fe;
  return merge_file;
}

// Write content of a Key_File struct to specified location
void write_key_file(Key_File key_file, const char *save_to_dir,
                    const char *file_name) {
  // Check if the directory exists
  DIR *dir = opendir(save_to_dir);
  if (dir) {
    closedir(dir);
  } else {
    errno = ENOENT;
    return;
  }
  // Create a file handle for the specified file
  char *save_to = combine_path_name(save_to_dir, file_name);
  FILE *kf = fopen(save_to, "w");
  if (kf == NULL) {
    errno = EPERM;
    return;
  }

  // Write to file
  for (int i = 0; i < key_file.length; i++) {
    if (!i || strcmp(key_file.file_entry[i - 1].group,
                     key_file.file_entry[i].group)) {
      if (i)
        fprintf(kf, "\n");
      if (strcmp(key_file.file_entry[i].group, "[]"))
        fprintf(kf, "%s\n", key_file.file_entry[i].group);
    }
    fprintf(kf, "%s%c%s\n", key_file.file_entry[i].key, key_file.delimiter,
            key_file.file_entry[i].value);
  }

  // Clean up
  free(save_to);
  fclose(kf);
}

// Wrapper function to perform the merge in one step
void merge_files(const char *save_to_dir, const char *file_name,
                 const char *etc_path, const char *usr_path,
                 const char delimiter, const char comment) {

  /* --- GET KEY FILES --- */

  char *usr_file_name = combine_path_name(usr_path, file_name);
  char *etc_file_name = combine_path_name(etc_path, file_name);

  Key_File usr_file = get_key_file(usr_file_name, delimiter, comment);
  Key_File etc_file = get_key_file(etc_file_name, delimiter, comment);

  /* --- MERGE KEY FILES --- */

  Key_File merged_file = merge_key_files(&usr_file, &etc_file);

  /* --- WRITE MERGED FILE --- */

  write_key_file(merged_file, save_to_dir, file_name);

  /* --- CLEAN UP --- */
  free(etc_file_name);
  free(usr_file_name);
  destroy(usr_file);
  destroy(etc_file);
  destroy_merged_file(merged_file);
}
