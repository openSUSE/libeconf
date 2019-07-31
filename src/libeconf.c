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

#include "../include/libeconf.h"

#include "../include/getfilecontents.h"
#include "../include/helpers.h"
#include "../include/mergefiles.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Process the file of the given file_name and save its contents into key_file
Key_File get_key_file(const char *file_name, char *delim,
                      const char comment) {
  Key_File read_file = {.comment = comment};

  // File handle for the given file_name
  FILE *kf = fopen(file_name, "rb");
  if (kf == NULL) {
    read_file.length = -ENOENT;
    return read_file;
  }

  read_file = fill_key_file(read_file, kf, delim);

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
      strcmp(usr_file->file_entry->group, "[]")) {
    merge_length = insert_nogroup(&fe, etc_file);
  }
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
  char *save_to = combine_strings(save_to_dir, file_name, '/');
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
                 char *delimiter, const char comment) {

  /* --- GET KEY FILES --- */

  char *usr_file_name = combine_strings(usr_path, file_name, '/');
  char *etc_file_name = combine_strings(etc_path, file_name, '/');

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

/* GETTER FUNCTIONS */
// TODO: Return values in error case

// TODO: Currently only works with a sorted Key_File. If a new
// key with an existing group is appended at the end the group
// will show twice. So the key file either needs to be sorted
// upon entering a new key or the function must ensure only
// unique values are returned.
char **econf_getGroups(Key_File kf, size_t *length) {
  size_t tmp = 0;
  bool *uniques = calloc(kf.length,sizeof(bool));
  for (int i = 0; i < kf.length; i++) {
    if (!i || strcmp(kf.file_entry[i].group, kf.file_entry[i - 1].group)) {
      uniques[i] = 1;
      tmp++;
    }
  }
  char **groups = calloc(tmp + 1, sizeof(char*));
  tmp = 0;
  for(int i = 0; i < kf.length; i++) {
    if (uniques[i]) { groups[tmp++] = strdup(kf.file_entry[i].group); }
  }

  if (length != NULL) { *length = tmp; }

  free(uniques);
  return groups;
}

// TODO: Same issue as with getGroups()
char **econf_getKeys(Key_File kf, const char *grp, size_t *length) {
  size_t tmp = 0;
  char *group = strdup(grp);
  group = addbrackets(group);
  bool *uniques = calloc(kf.length, sizeof(bool));
  for (int i = 0; i < kf.length; i++) {
    if (!strcmp(kf.file_entry[i].group, group) &&
        (!i || strcmp(kf.file_entry[i].key, kf.file_entry[i - 1].key))) {
      uniques[i] = 1;
      tmp++;
    }
  }
  char **keys = calloc(tmp + 1, sizeof(char*));
  for(int i = 0, tmp = 0; i < kf.length; i++) {
    if (uniques[i]) { keys[tmp++] = strdup(kf.file_entry[i].key); }
  }

  if (length != NULL) { *length = tmp; }

  free(uniques), free(group);
  return keys;
}

int32_t getIntValue(Key_File kf, char *group, char *key) {
  size_t num = find_key(kf, group, key);
  if (num != -1) return getIntValueNum(kf, num);
  return -1;
}

int64_t getInt64Value(Key_File kf, char *group, char *key) {
  size_t num = find_key(kf, group, key);
  if (num != -1) return getInt64ValueNum(kf, num);
  return -1;
}

uint32_t getUIntValue(Key_File kf, char *group, char *key) {
  size_t num = find_key(kf, group, key);
  if (num != -1) return getUIntValueNum(kf, num);
  return -1;
}

uint64_t getUInt64Value(Key_File kf, char *group, char *key) {
  size_t num = find_key(kf, group, key);
  if (num != -1) return getUInt64ValueNum(kf, num);
  return -1;
}

float getFloatValue(Key_File kf, char *group, char *key) {
  size_t num = find_key(kf, group, key);
  if (num != -1) return getFloatValueNum(kf, num);
  return -1;
}

double getDoubleValue(Key_File kf, char *group, char *key) {
  size_t num = find_key(kf, group, key);
  if (num != -1) return getDoubleValueNum(kf, num);
  return -1;
}

char *getStringValue(Key_File kf, char *group, char *key) {
  size_t num = find_key(kf, group, key);
  if (num != -1) return getStringValueNum(kf, num);
  return "";
}

bool getBoolValue(Key_File kf, char *group, char *key) {
  size_t num = find_key(kf, group, key);
  if (num != -1) return getBoolValueNum(kf, num);
  return -1;
}

/* SETTER FUNCTIONS */

void setIntValue(Key_File *kf, char *group, char *key, int64_t value) {
  setKeyValue(setIntValueNum, kf, group, key, &value);
}

void setUIntValue(Key_File kf, char *group, char *key, uint64_t value) {
  setKeyValue(setUIntValueNum, kf, group, key, &value);
}

void setFloatValue(Key_File *kf, char *group, char *key, float value) {
  setKeyValue(setFloatValueNum, kf, group, key, &value);
}

void setDoubleValue(Key_File *kf, char *group, char *key, double value) {
  setKeyValue(setDoubleValueNum, kf, group, key, &value);
}

void setStringValue(Key_File *kf, char *group, char *key, char *value) {
  setKeyValue(setStringValueNum, kf, group, key, value);
}

void setBoolValue(Key_File *kf, char *group, char *key, char *value) {
  setKeyValue(setBoolValueNum, kf, group, key, value);
}

/* --- DESTROY FUNCTIONS --- */

void afree(char** array) {
  char *tmp = (char*) array;
  while (*array)
    free(*array++);
  free(tmp);
}

// Free memory allocated by key_file
void destroy(Key_File key_file) {
  for (int i = 0; i < key_file.alloc_length; i++) {
    free(key_file.file_entry[i].group);
    free(key_file.file_entry[i].key);
    free(key_file.file_entry[i].value);
  }
  free(key_file.file_entry);
}

// Wrapper function to free memory of merged file
void destroy_merged_file(Key_File key_file) { free(key_file.file_entry); }

