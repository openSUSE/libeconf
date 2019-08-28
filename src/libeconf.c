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
Key_File *econf_get_key_file(const char *file_name, const char *delim,
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
    fclose (kf);
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
                                   const char *project_name,
				   const char *config_suffix,
                                   const char *delim, char comment,
                                   econf_err *error) {
  size_t size = 1;
  char *suffix;

  /* config_suffix must be provided and should not be "" */
  if (config_suffix == NULL || strlen (config_suffix) == 0)
    {
      if (error)
	*error = ECONF_ERROR;
      return NULL;
    }

  // Prepend a . to the config suffix if not provided
  if (config_suffix[0] == '.')
    suffix = strdup (config_suffix);
  else
    suffix = combine_strings("", config_suffix, '.');
  if (suffix == NULL)
    {
      if (error) *error = ECONF_NOMEM;
      return NULL;
    }

  char *file_name = combine_strings(project_name, &*(suffix + 1), '.');
  if (file_name == NULL)
    {
      if (error) *error = ECONF_NOMEM;
      return NULL;
    }

  // Get the list of directories to search for config files
  char **default_dirs = get_default_dirs(usr_conf_dir, etc_conf_dir);
  if (*default_dirs == NULL)
    {
      if (error) *error = ECONF_NOMEM;
      return NULL;
    }
  char **default_ptr = default_dirs;

  Key_File **key_files = malloc(size * sizeof(Key_File*));
  if (key_files == NULL)
    {
      if (error)
	*error = ECONF_NOMEM;
      return NULL;
    }
  while (*default_dirs) {
    // Check if the config file exists directly in the given config directory
    char *file_path = combine_strings(*default_dirs, file_name, '/');
    Key_File *key_file = econf_get_key_file(file_path, delim, comment, NULL);
    free(file_path);
    if(key_file) {
      key_file->on_merge_delete = 1;
      key_files[size - 1] = key_file;
      Key_File **tmp = realloc(key_files, ++size * sizeof(Key_File *));
      if (!tmp) {
        for (int i = 0; i < size - 1; i++) free(key_files[i]);
        free(key_files);
        if (error) *error = ECONF_NOMEM;
        return NULL;
      }
      key_files = tmp;
    }

    // Indicate which directories to look for
    // Gets expanded to:
    // "default_dirs/project_name/"
    // "default_dirs/project_name/conf.d/"
    // "default_dirs/project_name.conf.d/"
    // "default_dirs/project_name.d/"
    // in this order
    char *conf_dirs = " d . conf.d . conf.d / 0 ";
    char *project_path = combine_strings(*default_dirs, project_name, '/');
    /* XXX ENOMEM/NULL pointer check */
    // Check for '$suffix' files in config directories with the
    // given project name
    key_files = traverse_conf_dirs(key_files, conf_dirs, &size, project_path,
                                   suffix, delim, comment);
    /* XXX ENOMEM/NULL pointer check */
    free(project_path);

    // If /etc configuration exists igonre /usr config
    if ((size - 1) && (default_dirs == default_ptr)) { default_dirs++; }
    default_dirs++;
  }
  key_files[size - 1] = NULL;

  // Free allocated memory and return
  free(file_name);
  free(suffix);
  econf_destroy(default_ptr);

  // Merge the list of acquired key_files into merged_file
  Key_File *merged_file = merge_Key_Files(key_files, error);
  free(key_files);

  return merged_file;
}

// Write content of a Key_File struct to specified location
void econf_write_key_file(Key_File *key_file, const char *save_to_dir,
                          const char *file_name, econf_err *error) {
  if (!key_file) {
    if (error) *error = ECONF_ERROR;
    return;
  }
  // Check if the directory exists
  // XXX use stat instead of opendir
  DIR *dir = opendir(save_to_dir);
  if (dir) {
    closedir(dir);
  } else {
    if (error) *error = ECONF_NOFILE;
    return;
  }
  // Create a file handle for the specified file
  char *save_to = combine_strings(save_to_dir, file_name, '/');
  if (save_to == NULL) {
    if (error) *error = ECONF_NOMEM;
    return;
  }
  FILE *kf = fopen(save_to, "w");
  if (kf == NULL) {
    if (error) *error = ECONF_WRITEERROR;
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
// TODO: Currently only works with a sorted Key_File. If a new
// key with an existing group is appended at the end the group
// will show twice. So the key file either needs to be sorted
// upon entering a new key or the function must ensure only
// unique values are returned.
char **econf_getGroups(Key_File *kf, size_t *length, econf_err *error) {
  if (!kf) {
    if (error) *error = ECONF_ERROR;
    return NULL;
  }
  size_t tmp = 0;
  bool *uniques = calloc(kf->length,sizeof(bool));
  if (uniques == NULL) {
    if (error) *error = ECONF_NOMEM;
    return NULL;
  }
  for (int i = 0; i < kf->length; i++) {
    if ((!i || strcmp(kf->file_entry[i].group, kf->file_entry[i - 1].group)) &&
        strcmp(kf->file_entry[i].group, KEY_FILE_NULL_VALUE)) {
      uniques[i] = 1;
      tmp++;
    }
  }
  if (!tmp) { free(uniques); return NULL; }
  char **groups = calloc(tmp + 1, sizeof(char*));
  if (groups == NULL) {
    if (error) *error = ECONF_NOMEM;
    return NULL;
  }
  tmp = 0;
  for(int i = 0; i < kf->length; i++) {
    if (uniques[i]) { groups[tmp++] = strdup(kf->file_entry[i].group); }
  }

  if (length != NULL) { *length = tmp; }

  free(uniques);
  return groups;
}

// TODO: Same issue as with getGroups()
char **econf_getKeys(Key_File *kf, const char *grp, size_t *length, econf_err *error) {
  if (!kf) {
    if (error) *error = ECONF_ERROR;
    return NULL;
  }

  size_t tmp = 0;
  char *group = ((!grp || !*grp) ? strdup(KEY_FILE_NULL_VALUE) :
                 addbrackets(grp));
  if (group == NULL) {
    if (error) *error = ECONF_NOMEM;
    return NULL;
  }
  bool *uniques = calloc(kf->length, sizeof(bool));
  if (uniques == NULL) {
    free(group);
    if (error) *error = ECONF_NOMEM;
    return NULL;
  }
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
  if (keys == NULL) {
    if (error) *error = ECONF_NOMEM;
    free (uniques);
    return NULL;
  }
  for(int i = 0, tmp = 0; i < kf->length; i++) {
    if (uniques[i]) { keys[tmp++] = strdup(kf->file_entry[i].key); }
  }

  if (length != NULL) { *length = tmp; }

  free(uniques);
  return keys;
}

/* The econf_get*Value functions are identical except for result
   value type, so let's create them via a macro. */
#define econf_getValue(FCT_TYPE, TYPE)			      \
econf_err econf_get ## FCT_TYPE ## Value(Key_File *kf, const char *group, \
			     const char *key, TYPE *result) {	\
  if (!kf) \
    return ECONF_ERROR; \
\
  size_t num; \
  econf_err error = find_key(*kf, group, key, &num);	\
  if (error) \
    return error; \
  return get ## FCT_TYPE ## ValueNum(*kf, num, result);	\
}

econf_getValue(Int, int32_t)
econf_getValue(Int64, int64_t)
econf_getValue(UInt, uint32_t)
econf_getValue(UInt64, uint64_t)
econf_getValue(Float, float)
econf_getValue(Double, double)
econf_getValue(String, char *)
econf_getValue(Bool, bool)

/* SETTER FUNCTIONS */
/* The econf_set*Value functions are identical except for set
   value type, so let's create them via a macro. */
#define libeconf_setValue(TYPE, VALTYPE, VALARG) \
econf_err econf_set ## TYPE ## Value(Key_File *kf, const char *group,		\
  const char *key, VALTYPE value) {	\
  if (!kf) \
    return ECONF_ERROR; \
  return setKeyValue(set ## TYPE ## ValueNum, kf, group, key, VALARG); \
}

libeconf_setValue(Int, int32_t, &value)
libeconf_setValue(Int64, int64_t, &value)
libeconf_setValue(UInt, uint32_t, &value)
libeconf_setValue(UInt64, uint64_t, &value)
libeconf_setValue(Float, float, &value)
libeconf_setValue(Double, double, &value)
libeconf_setValue(String, const char *, value)
libeconf_setValue(Bool, const char *, value)

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
