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

#include "../include/libeconf.h"

#include "../include/defines.h"
#include "../include/getfilecontents.h"
#include "../include/helpers.h"
#include "../include/keyfile.h"
#include "../include/mergefiles.h"

#include <dirent.h>
#include <stdio.h>
#include <string.h>

// Create a new Key_File. Allocation is based on KEY_FILE_KEY_FILE_KEY_FILE_DEFAULT_LENGTH
// defined in include/defines.h
Key_File *econf_newKeyFile(char delimiter, char comment) {
  Key_File *key_file = malloc(sizeof(Key_File));

  key_file->alloc_length = KEY_FILE_DEFAULT_LENGTH;
  key_file->length = 0;
  key_file->delimiter = delimiter;
  key_file->comment = comment;

  key_file->file_entry = malloc(KEY_FILE_DEFAULT_LENGTH * sizeof(struct file_entry));

  for (size_t i = 0; i < KEY_FILE_DEFAULT_LENGTH; i++) {
    initialize(key_file, i);
  }

  return key_file;
}

Key_File *econf_newIniFile() {
  return econf_newKeyFile('=', '#');
}

// Process the file of the given file_name and save its contents into key_file
Key_File *econf_get_key_file(const char *file_name, char *delim,
                             const char comment) {
  // Get absolute path if not provided
  errno = 0;
  char *absolute_path = get_absolute_path(file_name);
  if(errno != 0) return NULL;

  // File handle for the given file_name
  FILE *kf = fopen(absolute_path, "rb");
  free(absolute_path);
  if (kf == NULL) {
    return NULL;
  }

  Key_File *read_file = malloc(sizeof(Key_File));
  read_file->comment = comment;

  fill_key_file(read_file, kf, delim);
  read_file->on_merge_delete = 0;
  fclose(kf);

  if(!read_file->length) {
    errno = ENODATA;
    econf_destroy(read_file);
    return NULL;
  }

  return read_file;
}

// Merge the contents of two key files
Key_File *econf_merge_key_files(Key_File *usr_file, Key_File *etc_file) {
  Key_File *merge_file = malloc(sizeof(Key_File));
  merge_file->delimiter = usr_file->delimiter;
  merge_file->comment = usr_file->comment;
  struct file_entry *fe =
      malloc((etc_file->length + etc_file->length) * sizeof(struct file_entry));

  size_t merge_length = 0;

  if (!strcmp(etc_file->file_entry->group, KEY_FILE_NULL_VALUE) &&
      strcmp(usr_file->file_entry->group, KEY_FILE_NULL_VALUE)) {
    merge_length = insert_nogroup(&fe, etc_file);
  }
  merge_length = merge_existing_groups(&fe, usr_file, etc_file, merge_length);
  merge_file->length = add_new_groups(&fe, usr_file, etc_file, merge_length);

  merge_file->file_entry = fe;
  return merge_file;
}

// Write content of a Key_File struct to specified location
void econf_write_key_file(Key_File *key_file, const char *save_to_dir,
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
  for (int i = 0; i < key_file->length; i++) {
    if (!i || strcmp(key_file->file_entry[i - 1].group,
                     key_file->file_entry[i].group)) {
      if (i)
        fprintf(kf, "\n");
      if (strcmp(key_file->file_entry[i].group, KEY_FILE_NULL_VALUE))
        fprintf(kf, "%s\n", key_file->file_entry[i].group);
    }
    fprintf(kf, "%s%c%s\n", key_file->file_entry[i].key, key_file->delimiter,
            key_file->file_entry[i].value);
  }

  // Clean up
  free(save_to);
  fclose(kf);
}

// Wrapper function to perform the merge in one step
void econf_merge_files(const char *save_to_dir, const char *file_name,
                 const char *etc_path, const char *usr_path,
                 char *delimiter, const char comment) {

  /* --- GET KEY FILES --- */

  char *usr_file_name = combine_strings(usr_path, file_name, '/');
  char *etc_file_name = combine_strings(etc_path, file_name, '/');

  Key_File *usr_file = econf_get_key_file(usr_file_name, delimiter, comment);
  Key_File *etc_file = econf_get_key_file(etc_file_name, delimiter, comment);

  /* --- MERGE KEY FILES --- */

  Key_File *merged_file = econf_merge_key_files(usr_file, etc_file);

  /* --- WRITE MERGED FILE --- */

  econf_write_key_file(merged_file, save_to_dir, file_name);

  /* --- CLEAN UP --- */
  free(etc_file_name);
  free(usr_file_name);
  econf_destroy(usr_file);
  econf_destroy(etc_file);
  econf_destroy_merged_file(merged_file);
}

/* GETTER FUNCTIONS */
// TODO: Return values in error case

// TODO: Currently only works with a sorted Key_File. If a new
// key with an existing group is appended at the end the group
// will show twice. So the key file either needs to be sorted
// upon entering a new key or the function must ensure only
// unique values are returned.
char **econf_getGroups(Key_File *kf, size_t *length) {
  size_t tmp = 0;
  bool *uniques = calloc(kf->length,sizeof(bool));
  for (int i = 0; i < kf->length; i++) {
    if (!i || strcmp(kf->file_entry[i].group, kf->file_entry[i - 1].group)) {
      uniques[i] = 1;
      tmp++;
    }
  }
  char **groups = calloc(tmp + 1, sizeof(char*));
  tmp = 0;
  for(int i = 0; i < kf->length; i++) {
    if (uniques[i]) { groups[tmp++] = strdup(kf->file_entry[i].group); }
  }

  if (length != NULL) { *length = tmp; }

  free(uniques);
  return groups;
}

// TODO: Same issue as with getGroups()
char **econf_getKeys(Key_File *kf, const char *grp, size_t *length) {
  size_t tmp = 0;
  char *group = strdup(grp);
  group = addbrackets(group);
  bool *uniques = calloc(kf->length, sizeof(bool));
  for (int i = 0; i < kf->length; i++) {
    if (!strcmp(kf->file_entry[i].group, group) &&
        (!i || strcmp(kf->file_entry[i].key, kf->file_entry[i - 1].key))) {
      uniques[i] = 1;
      tmp++;
    }
  }
  char **keys = calloc(tmp + 1, sizeof(char*));
  for(int i = 0, tmp = 0; i < kf->length; i++) {
    if (uniques[i]) { keys[tmp++] = strdup(kf->file_entry[i].key); }
  }

  if (length != NULL) { *length = tmp; }

  free(uniques), free(group);
  return keys;
}

int32_t econf_getIntValue(Key_File *kf, char *group, char *key) {
  size_t num = find_key(*kf, group, key);
  if (num != -1) return getIntValueNum(*kf, num);
  return -1;
}

int64_t econf_getInt64Value(Key_File *kf, char *group, char *key) {
  size_t num = find_key(*kf, group, key);
  if (num != -1) return getInt64ValueNum(*kf, num);
  return -1;
}

uint32_t econf_getUIntValue(Key_File *kf, char *group, char *key) {
  size_t num = find_key(*kf, group, key);
  if (num != -1) return getUIntValueNum(*kf, num);
  return -1;
}

uint64_t econf_getUInt64Value(Key_File *kf, char *group, char *key) {
  size_t num = find_key(*kf, group, key);
  if (num != -1) return getUInt64ValueNum(*kf, num);
  return -1;
}

float econf_getFloatValue(Key_File *kf, char *group, char *key) {
  size_t num = find_key(*kf, group, key);
  if (num != -1) return getFloatValueNum(*kf, num);
  return -1;
}

double econf_getDoubleValue(Key_File *kf, char *group, char *key) {
  size_t num = find_key(*kf, group, key);
  if (num != -1) return getDoubleValueNum(*kf, num);
  return -1;
}

char *econf_getStringValue(Key_File *kf, char *group, char *key) {
  size_t num = find_key(*kf, group, key);
  if (num != -1) return getStringValueNum(*kf, num);
  return "";
}

bool econf_getBoolValue(Key_File *kf, char *group, char *key) {
  size_t num = find_key(*kf, group, key);
  if (num != -1) return getBoolValueNum(*kf, num);
  return -1;
}

/* SETTER FUNCTIONS */

void econf_setIntValue(Key_File *kf, char *group, char *key, int64_t value) {
  setKeyValue(setIntValueNum, kf, group, key, &value);
}

void econf_setUIntValue(Key_File *kf, char *group, char *key, uint64_t value) {
  setKeyValue(setUIntValueNum, kf, group, key, &value);
}

void econf_setFloatValue(Key_File *kf, char *group, char *key, float value) {
  setKeyValue(setFloatValueNum, kf, group, key, &value);
}

void econf_setDoubleValue(Key_File *kf, char *group, char *key, double value) {
  setKeyValue(setDoubleValueNum, kf, group, key, &value);
}

void econf_setStringValue(Key_File *kf, char *group, char *key, char *value) {
  setKeyValue(setStringValueNum, kf, group, key, value);
}

void econf_setBoolValue(Key_File *kf, char *group, char *key, char *value) {
  setKeyValue(setBoolValueNum, kf, group, key, value);
}

/* --- DESTROY FUNCTIONS --- */

void econf_afree(char** array) {
  char *tmp = (char*) array;
  while (*array)
    free(*array++);
  free(tmp);
}

// Free memory allocated by key_file
void econf_destroy(Key_File *key_file) {
  for (int i = 0; i < key_file->alloc_length; i++) {
    free(key_file->file_entry[i].group);
    free(key_file->file_entry[i].key);
    free(key_file->file_entry[i].value);
  }
  free(key_file->file_entry);
  free(key_file);
}

// Wrapper function to free memory of merged file
void econf_destroy_merged_file(Key_File *key_file) {
  free(key_file->file_entry);
  free(key_file);
}
