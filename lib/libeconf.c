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

#include "libeconf.h"

#include "defines.h"
#include "getfilecontents.h"
#include "helpers.h"
#include "keyfile.h"
#include "mergefiles.h"

#include <dirent.h>
#include <stdio.h>
#include <string.h>

// Create a new econf_file. Allocation is based on
// KEY_FILE_KEY_FILE_KEY_FILE_DEFAULT_LENGTH
// defined in include/defines.h
econf_err
econf_newKeyFile(econf_file **result, char delimiter, char comment)
{
  econf_file *key_file = calloc(1, sizeof(econf_file));

  if (key_file == NULL)
    return ECONF_NOMEM;

  key_file->alloc_length = KEY_FILE_DEFAULT_LENGTH;
  key_file->length = 0;
  key_file->delimiter = delimiter;
  key_file->comment = comment;

  key_file->file_entry = malloc(KEY_FILE_DEFAULT_LENGTH * sizeof(struct file_entry));
  if (key_file->file_entry == NULL)
    {
      free (key_file);
      return ECONF_NOMEM;
    }

  for (size_t i = 0; i < KEY_FILE_DEFAULT_LENGTH; i++)
    initialize(key_file, i);

  *result = key_file;

  return ECONF_SUCCESS;
}

econf_err econf_newIniFile(econf_file **result) {
  return econf_newKeyFile(result, '=', '#');
}

// Process the file of the given file_name and save its contents into key_file
econf_err econf_readFile(econf_file **key_file, const char *file_name,
			     const char *delim, const char *comment)
{
  econf_err t_err;

  if (key_file == NULL || file_name == NULL || delim == NULL)
    return ECONF_ERROR;

  // Get absolute path if not provided
  char *absolute_path = get_absolute_path(file_name, &t_err);
  if (absolute_path == NULL)
    return t_err;

  *key_file = calloc(1, sizeof(econf_file));
  if (*key_file == NULL) {
    free (absolute_path);
    return ECONF_NOMEM;
  }

  if (comment && *comment)
    (*key_file)->comment = comment[0];
  else
    (*key_file)->comment = '#';

  t_err = read_file(*key_file, absolute_path, delim, comment);
  free (absolute_path);

  if(t_err) {
    econf_free(*key_file);
    *key_file = NULL;
    return t_err;
  }

  return ECONF_SUCCESS;
}

// Merge the contents of two key files
econf_err econf_mergeFiles(econf_file **merged_file, econf_file *usr_file, econf_file *etc_file)
{
  if (merged_file == NULL || usr_file == NULL || etc_file == NULL)
    return ECONF_ERROR;

  *merged_file = calloc(1, sizeof(econf_file));
  if (*merged_file == NULL)
    return ECONF_NOMEM;

  (*merged_file)->delimiter = usr_file->delimiter;
  (*merged_file)->comment = usr_file->comment;
  struct file_entry *fe =
      malloc((etc_file->length + usr_file->length) * sizeof(struct file_entry));
  if (fe == NULL)
    {
      free (*merged_file);
      *merged_file = NULL;
      return ECONF_NOMEM;
    }

  size_t merge_length = 0;

  if ((etc_file->file_entry == NULL ||
       !strcmp(etc_file->file_entry->group, KEY_FILE_NULL_VALUE)) &&
      (usr_file->file_entry == NULL ||
       strcmp(usr_file->file_entry->group, KEY_FILE_NULL_VALUE))) {
    merge_length = insert_nogroup(&fe, etc_file);
  }
  merge_length = merge_existing_groups(&fe, usr_file, etc_file, merge_length);
  merge_length = add_new_groups(&fe, usr_file, etc_file, merge_length);
  (*merged_file)->length = merge_length;
  (*merged_file)->alloc_length = merge_length;

  (*merged_file)->file_entry = fe;
  return ECONF_SUCCESS;
}

econf_err econf_readDirs(econf_file **result,
				   const char *dist_conf_dir,
                                   const char *etc_conf_dir,
                                   const char *project_name,
                                   const char *config_suffix,
                                   const char *delim,
				   const char *comment)
{
  size_t size = 0;
  const char *suffix, *default_dirs[3] = {NULL, NULL, NULL};
  char *distfile, *etcfile, *cp;
  econf_file **key_files, *key_file;
  econf_err error;

  /* config_suffix must be provided and should not be "" */
  if (config_suffix == NULL || strlen (config_suffix) == 0 ||
      project_name == NULL || strlen (project_name) == 0 || delim == NULL)
    return ECONF_ERROR;

  // Prepend a . to the config suffix if not provided
  if (config_suffix[0] == '.')
    suffix = config_suffix;
  else
    {
      cp = alloca (strlen(config_suffix) + 2);
      cp[0] = '.';
      strcpy(cp+1, config_suffix);
      suffix = cp;
    }

  /* create file names for etc and distribution config */
  if (dist_conf_dir != NULL)
    {
      distfile = alloca(strlen (dist_conf_dir) + strlen (project_name) +
			strlen (suffix) + 2);

      cp = stpcpy (distfile, dist_conf_dir);
      *cp++ = '/';
      cp = stpcpy (cp, project_name);
      stpcpy (cp, suffix);
    }
  else
    distfile = NULL;

  if (etc_conf_dir != NULL)
    {
      etcfile = alloca(strlen (etc_conf_dir) + strlen (project_name) +
		       strlen (suffix) + 2);

      cp = stpcpy (etcfile, etc_conf_dir);
      *cp++ = '/';
      cp = stpcpy (cp, project_name);
      stpcpy (cp, suffix);
    }
  else
    etcfile = NULL;

  if (etcfile)
    {
      error = econf_readFile(&key_file, etcfile, delim, comment);
      if (error && error != ECONF_NOFILE)
	return error;
    }

  if (etcfile && !error) {
    /* /etc/<project_name>.<suffix> does exist, ignore /usr */
    default_dirs[0] = etc_conf_dir;
    size = 1;
  } else {
    /* /etc/<project_name>.<suffix> does not exist, so read /usr/etc
       and merge all *.d files. */
    if (distfile)
      {
	error = econf_readFile(&key_file, distfile, delim, comment);
	if (error && error != ECONF_NOFILE)
	  return error;
      }

    if (distfile && !error) /* /usr/etc/<project_name>.<suffix> does exist */
      size = 1;

    default_dirs[0] = dist_conf_dir;
    default_dirs[1] = etc_conf_dir;
  }

  /* XXX Re-add get_default_dirs in a reworked version, which
     adds additional directories to look at, e.g. XDG or home directory */

  /* create space to store the econf_files for merging */
  key_files = calloc((++size), sizeof(econf_file*));
  if (key_files == NULL)
    return ECONF_NOMEM;

  if (size == 2)
    {
      key_file->on_merge_delete = 1;
      key_files[0] = key_file;
    }

  int i = 0;
  while (default_dirs[i]) {
    /*
      Indicate which directories to look for. The order is:
       "default_dirs/project_name.suffix.d/"

       XXX make this configureable:
       "default_dirs/project_name/conf.d/"
       "default_dirs/project_name.d/"
       "default_dirs/project_name/"
    */
    const char *conf_dirs[] = {  NULL, /* "/conf.d/", ".d/", "/", */ NULL};
    char *project_path = combine_strings(default_dirs[i], project_name, '/');
    char *suffix_d = malloc (strlen(suffix) + 4); /* + strlen(".d/") */
    /* XXX ENOMEM/NULL pointer check */
    cp = stpcpy(suffix_d, suffix);
    stpcpy(cp, ".d/");
    conf_dirs[0] = suffix_d;
    key_files = traverse_conf_dirs(key_files, conf_dirs, &size, project_path,
                                   suffix, delim, comment);
    /* XXX ENOMEM/NULL pointer check */
    free(suffix_d);
    free(project_path);
    i++;
  }
  key_files[size - 1] = NULL;

  // Merge the list of acquired key_files into merged_file
  error = merge_econf_files(key_files, result);
  free(key_files);

  if (size == 1)
    return ECONF_NOFILE;
  else
    return error;
}

// Write content of a econf_file struct to specified location
econf_err econf_writeFile(econf_file *key_file, const char *save_to_dir,
			       const char *file_name) {
  if (!key_file)
    return ECONF_ERROR;

  // Check if the directory exists
  // XXX use stat instead of opendir
  DIR *dir = opendir(save_to_dir);
  if (dir)
    closedir(dir);
  else
    return ECONF_NOFILE;

  // Create a file handle for the specified file
  char *save_to = combine_strings(save_to_dir, file_name, '/');
  if (save_to == NULL)
    return ECONF_NOMEM;

  FILE *kf = fopen(save_to, "w");
  if (kf == NULL)
    return ECONF_WRITEERROR;

  // Write to file
  for (size_t i = 0; i < key_file->length; i++) {
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
  return ECONF_SUCCESS;
}

/* GETTER FUNCTIONS */
// TODO: Currently only works with a sorted econf_file. If a new
// key with an existing group is appended at the end the group
// will show twice. So the key file either needs to be sorted
// upon entering a new key or the function must ensure only
// unique values are returned.
econf_err
econf_getGroups(econf_file *kf, size_t *length, char ***groups)
{
  if (!kf || groups == NULL)
    return ECONF_ERROR;

  size_t tmp = 0;
  bool *uniques = calloc(kf->length,sizeof(bool));
  if (uniques == NULL)
    return ECONF_NOMEM;

  for (size_t i = 0; i < kf->length; i++) {
    if ((!i || strcmp(kf->file_entry[i].group, kf->file_entry[i - 1].group)) &&
        strcmp(kf->file_entry[i].group, KEY_FILE_NULL_VALUE)) {
      uniques[i] = 1;
      tmp++;
    }
  }
  if (!tmp) {
    free(uniques);
    return ECONF_NOGROUP;
  }
  *groups = calloc(tmp + 1, sizeof(char*));
  if (*groups == NULL)
    return ECONF_NOMEM;

  tmp = 0;
  for (size_t i = 0; i < kf->length; i++)
    if (uniques[i])
      (*groups)[tmp++] = strdup(kf->file_entry[i].group);

  if (length != NULL)
    *length = tmp;

  free(uniques);
  return ECONF_SUCCESS;
}

// TODO: Same issue as with getGroups()
econf_err
econf_getKeys(econf_file *kf, const char *grp, size_t *length, char ***keys)
{
  if (!kf)
    return ECONF_ERROR;

  size_t tmp = 0;
  char *group = ((!grp || !*grp) ? strdup(KEY_FILE_NULL_VALUE) :
                 addbrackets(grp));
  if (group == NULL)
    return ECONF_NOMEM;

  bool *uniques = calloc(kf->length, sizeof(bool));
  if (uniques == NULL)
    {
      free(group);
      return ECONF_NOMEM;
    }
  for (size_t i = 0; i < kf->length; i++) {
    if (!strcmp(kf->file_entry[i].group, group) &&
        (!i || strcmp(kf->file_entry[i].key, kf->file_entry[i - 1].key))) {
      uniques[i] = 1;
      tmp++;
    }
  }
  free(group);
  if (!tmp)
    {
      free (uniques);
      return ECONF_NOKEY;
    }
  *keys = calloc(tmp + 1, sizeof(char*));
  if (*keys == NULL) {
    free (uniques);
    return ECONF_NOMEM;
  }

  for (size_t i = 0, j = 0; i < kf->length; i++)
    if (uniques[i])
      (*keys)[j++] = strdup(kf->file_entry[i].key);

  if (length != NULL)
    *length = tmp;

  free(uniques);
  return ECONF_SUCCESS;
}

/* The econf_get*Value functions are identical except for result
   value type, so let's create them via a macro. */
#define econf_getValue(FCT_TYPE, TYPE)			      \
econf_err econf_get ## FCT_TYPE ## Value(econf_file *kf, const char *group, \
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
econf_err econf_set ## TYPE ## Value(econf_file *kf, const char *group,		\
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

void econf_freeArray(char** array) {
  if (!array) { return; }
  char *tmp = (char*) array;
  while (*array)
    free(*array++);
  free(tmp);
}

// Free memory allocated by key_file
void econf_freeFile(econf_file *key_file) {
  if (!key_file)
    return;

  for (size_t i = 0; i < key_file->alloc_length; i++) {
    if (key_file->file_entry[i].group)
      free(key_file->file_entry[i].group);
    if (key_file->file_entry[i].key)
      free(key_file->file_entry[i].key);
    if (key_file->file_entry[i].value)
      free(key_file->file_entry[i].value);
  }

  if (key_file->file_entry)
    free(key_file->file_entry);
  if (key_file->path)
    free(key_file->path);

  free(key_file);
}
