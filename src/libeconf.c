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

#include <errno.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>

// Create a new Key_File. Allocation is based on KEY_FILE_KEY_FILE_KEY_FILE_DEFAULT_LENGTH
// defined in include/defines.h
Key_File *econf_newKeyFile(char delimiter, char comment, econf_err *error) {
  Key_File *key_file = malloc(sizeof(Key_File));

  if (key_file == NULL)
    {
      if (error)
	*error = ECONF_NOMEM;
      return NULL;
    }
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

Key_File *econf_newIniFile(econf_err *error) {
  return econf_newKeyFile('=', '#', error);
}

// Process the file of the given file_name and save its contents into key_file
Key_File *econf_get_key_file(const char *file_name, char *delim,
                             const char comment, econf_err *error) {
  econf_err t_err;

  // Get absolute path if not provided
  char *absolute_path = get_absolute_path(file_name, error);
  if (absolute_path == NULL)
    return NULL;

  // File handle for the given file_name
  FILE *kf = fopen(absolute_path, "rb");
  free(absolute_path);
  if (kf == NULL) {
    if (error)
      *error = ECONF_NOFILE;
    return NULL;
  }

  Key_File *read_file = malloc(sizeof(Key_File));
  if (read_file == NULL) {
    if (error)
      *error = ECONF_NOMEM;
    return NULL;
  }

  read_file->comment = comment;

  t_err = fill_key_file(read_file, kf, delim);
  read_file->on_merge_delete = 0;
  fclose(kf);

  if(t_err) {
    if (error)
      *error = t_err;
    econf_destroy(read_file);
    return NULL;
  }

  return read_file;
}

// Merge the contents of two key files
Key_File *econf_merge_key_files(Key_File *usr_file, Key_File *etc_file, econf_err *error) {
  if (usr_file == NULL || etc_file == NULL) {
    if (error)
      *error = ECONF_ERROR;
    return NULL;
  }

  Key_File *merge_file = malloc(sizeof(Key_File));
  if (merge_file == NULL)
    {
      if (error)
	*error = ECONF_NOMEM;
      return NULL;
    }

  merge_file->delimiter = usr_file->delimiter;
  merge_file->comment = usr_file->comment;
  struct file_entry *fe =
      malloc((etc_file->length + usr_file->length) * sizeof(struct file_entry));
  if (fe == NULL)
    {
      free (merge_file);
      if (error)
	*error = ECONF_NOMEM;
      return NULL;
    }

  size_t merge_length = 0;

  if (!strcmp(etc_file->file_entry->group, KEY_FILE_NULL_VALUE) &&
      strcmp(usr_file->file_entry->group, KEY_FILE_NULL_VALUE)) {
    merge_length = insert_nogroup(&fe, etc_file);
  }
  merge_length = merge_existing_groups(&fe, usr_file, etc_file, merge_length);
  merge_length = add_new_groups(&fe, usr_file, etc_file, merge_length);
  merge_file->length = merge_length;
  merge_file->alloc_length = merge_length;

  merge_file->file_entry = fe;
  return merge_file;
}

Key_File *econf_get_conf_from_dirs(const char *usr_conf_dir,
                                   const char *etc_conf_dir,
                                   char *project_name, char *config_suffix,
                                   char *delim, char comment,
                                   econf_err *error) {
  size_t size = 1;
  Key_File **key_files = malloc(size * sizeof(Key_File*));
  if (key_files == NULL)
    {
      if (error)
	*error = ECONF_NOMEM;
      return NULL;
    }

  /* config_suffix must be provided and should not be "" */
  if (config_suffix == NULL || strlen (config_suffix) == 0)
    {
      if (error)
	*error = ECONF_ERROR;
      return NULL;
    }

  // Prepend a . to the config suffix if not provided
  if (config_suffix[0] == '.')
    config_suffix = strdup (config_suffix);
  else
    config_suffix = combine_strings("", config_suffix, '.');
  if (config_suffix == NULL)
    {
      if (error) *error = ECONF_NOMEM;
      return NULL;
    }

  char *file_name = combine_strings(project_name, &*(config_suffix + 1), '.');
  if (file_name == NULL)
    {
      if (error) *error = ECONF_NOMEM;
      return NULL;
    }

  // Get the list of directories to search for config files
  char **default_dirs = get_default_dirs(usr_conf_dir, etc_conf_dir);
#if 0
  if (default_dirs == NULL)
    {
      if (error) *error = ECONF_NOMEM;
      return NULL;
    }
#endif
  char **default_ptr = default_dirs, *project_path;

  Key_File *key_file;
  while (*default_dirs) {
    project_path = combine_strings(*default_dirs, project_name, '/');

    // Check if the config file exists directly in the given config directory
    char *file_path = combine_strings(*default_dirs, file_name, '/');
    key_file = econf_get_key_file(file_path, delim, comment, NULL);
    free(file_path);
    if(key_file) {
      key_file->on_merge_delete = 1;
      key_files[size - 1] = key_file;
      key_files = realloc(key_files, ++size * sizeof(Key_File *));
      /* XXX ENOMEM check */
    }

    // Indicate which directories to look for
    // Gets expanded to:
    // "default_dirs/project_name/"
    // "default_dirs/project_name/conf.d/"
    // "default_dirs/project_name.conf.d/"
    // "default_dirs/project_name.d/"
    // in this order
    char *conf_dirs = " d . conf.d . conf.d / 0 ";
    // Check for '$config_suffix' files in config directories with the
    // given project name
    key_files = traverse_conf_dirs(key_files, conf_dirs, &size, project_path,
                                   config_suffix, delim, comment);

    free(project_path);

    // If /etc configuration exists igonre /usr config
    if ((size - 1) && (default_dirs == default_ptr)) { default_dirs++; }
    default_dirs++;
  }
  key_files[size - 1] = NULL;

  // Free allocated memory and return
  free(file_name); free(config_suffix);
  econf_destroy(default_ptr);

  // Merge the list of acquired key_files into merged_file
  Key_File *merged_file = merge_Key_Files(key_files);
  free(key_files);

  return merged_file;
}

// Write content of a Key_File struct to specified location
void econf_write_key_file(Key_File *key_file, const char *save_to_dir,
                          const char *file_name) {
  if (!key_file) { errno = ENODATA; return; }
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

/* GETTER FUNCTIONS */
// TODO: Return values in error case

// TODO: Currently only works with a sorted Key_File. If a new
// key with an existing group is appended at the end the group
// will show twice. So the key file either needs to be sorted
// upon entering a new key or the function must ensure only
// unique values are returned.
char **econf_getGroups(Key_File *kf, size_t *length) {
  if (!kf) { errno = ENODATA; return NULL; }
  size_t tmp = 0;
  bool *uniques = calloc(kf->length,sizeof(bool));
  for (int i = 0; i < kf->length; i++) {
    if ((!i || strcmp(kf->file_entry[i].group, kf->file_entry[i - 1].group)) &&
        strcmp(kf->file_entry[i].group, KEY_FILE_NULL_VALUE)) {
      uniques[i] = 1;
      tmp++;
    }
  }
  if (!tmp) { free(uniques); return NULL; }
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
  if (!kf) { errno = ENODATA; return NULL; }
  size_t tmp = 0;
  char *group = ((!grp || !*grp) ? strdup(KEY_FILE_NULL_VALUE) :
                 addbrackets(strdup(grp)));
  bool *uniques = calloc(kf->length, sizeof(bool));
  for (int i = 0; i < kf->length; i++) {
    if (!strcmp(kf->file_entry[i].group, group) &&
        (!i || strcmp(kf->file_entry[i].key, kf->file_entry[i - 1].key))) {
      uniques[i] = 1;
      tmp++;
    }
  }
  free(group);
  if (!tmp) { free(uniques); return NULL; }
  char **keys = calloc(tmp + 1, sizeof(char*));
  for(int i = 0, tmp = 0; i < kf->length; i++) {
    if (uniques[i]) { keys[tmp++] = strdup(kf->file_entry[i].key); }
  }

  if (length != NULL) { *length = tmp; }

  free(uniques);
  return keys;
}

int32_t econf_getIntValue(Key_File *kf, char *group, char *key) {
  if (!kf) { errno = ENODATA; return -1; }
  size_t num = find_key(*kf, group, key);
  if (num == -1) { errno = ENOKEY; return -1; }
  return getIntValueNum(*kf, num);
}

int64_t econf_getInt64Value(Key_File *kf, char *group, char *key) {
  if (!kf) { errno = ENODATA; return -1; }
  size_t num = find_key(*kf, group, key);
  if (num == -1) { errno = ENOKEY; return -1; }
  return getInt64ValueNum(*kf, num);
}

uint32_t econf_getUIntValue(Key_File *kf, char *group, char *key) {
  if (!kf) { errno = ENODATA; return -1; }
  size_t num = find_key(*kf, group, key);
  if (num == -1) { errno = ENOKEY; return -1; }
  return getUIntValueNum(*kf, num);
}

uint64_t econf_getUInt64Value(Key_File *kf, char *group, char *key) {
  if (!kf) { errno = ENODATA; return -1; }
  size_t num = find_key(*kf, group, key);
  if (num == -1) { errno = ENOKEY; return -1; }
  return getUInt64ValueNum(*kf, num);
}

float econf_getFloatValue(Key_File *kf, char *group, char *key) {
  if (!kf) { errno = ENODATA; return -1; }
  size_t num = find_key(*kf, group, key);
  if (num == -1) { errno = ENOKEY; return -1; }
  return getFloatValueNum(*kf, num);
}

double econf_getDoubleValue(Key_File *kf, char *group, char *key) {
  if (!kf) { errno = ENODATA; return -1; }
  size_t num = find_key(*kf, group, key);
  if (num == -1) { errno = ENOKEY; return -1; }
  return getDoubleValueNum(*kf, num);
}

char *econf_getStringValue(Key_File *kf, char *group, char *key) {
  if (!kf) { errno = ENODATA; return NULL; }
  size_t num = find_key(*kf, group, key);
  if (num == -1) { errno = ENOKEY; return NULL; }
  return getStringValueNum(*kf, num);
}

bool econf_getBoolValue(Key_File *kf, char *group, char *key) {
  if (!kf) { errno = ENODATA; return 0; }
  size_t num = find_key(*kf, group, key);
  if (num == -1) { errno = ENOKEY; return 0; }
  return getBoolValueNum(*kf, num);
}

/* SETTER FUNCTIONS */

void econf_setIntValue(Key_File *kf, char *group, char *key, int64_t value) {
  if (!kf) { errno = ENODATA; return; }
  setKeyValue(setIntValueNum, kf, group, key, &value);
}

void econf_setUIntValue(Key_File *kf, char *group, char *key, uint64_t value) {
  if (!kf) { errno = ENODATA; return; }
  setKeyValue(setUIntValueNum, kf, group, key, &value);
}

void econf_setFloatValue(Key_File *kf, char *group, char *key, float value) {
  if (!kf) { errno = ENODATA; return; }
  setKeyValue(setFloatValueNum, kf, group, key, &value);
}

void econf_setDoubleValue(Key_File *kf, char *group, char *key, double value) {
  if (!kf) { errno = ENODATA; return; }
  setKeyValue(setDoubleValueNum, kf, group, key, &value);
}

void econf_setStringValue(Key_File *kf, char *group, char *key, char *value) {
  if (!kf) { errno = ENODATA; return; }
  setKeyValue(setStringValueNum, kf, group, key, value);
}

void econf_setBoolValue(Key_File *kf, char *group, char *key, char *value) {
  if (!kf) { errno = ENODATA; return; }
  setKeyValue(setBoolValueNum, kf, group, key, value);
}

/* --- DESTROY FUNCTIONS --- */

void econf_char_a_destroy(char** array) {
  if (!array) { return; }
  char *tmp = (char*) array;
  while (*array)
    free(*array++);
  free(tmp);
}

// Free memory allocated by key_file
void econf_Key_File_destroy(Key_File *key_file) {
  if (!key_file) { return; }
  for (int i = 0; i < key_file->alloc_length; i++) {
    free(key_file->file_entry[i].group);
    free(key_file->file_entry[i].key);
    free(key_file->file_entry[i].value);
  }
  free(key_file->file_entry);
  free(key_file);
}
