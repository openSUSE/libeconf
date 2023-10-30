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
#include "readconfig.h"

#include <libgen.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

// Checking file permissions, uid, group,...
static bool file_owner_set = false;
static uid_t file_owner = 0;
static bool file_group_set = false;
static gid_t file_group = 0;
static bool file_permissions_set = false;
static mode_t file_perms_file;
static mode_t file_perms_dir;
bool allow_follow_symlinks = true;
// configuration directories format
char **conf_dirs = {NULL}; // see econf_set_conf_dirs
int conf_count = 0;

void econf_requireOwner(uid_t owner)
{
  file_owner_set = true;
  file_owner = owner;
}

void econf_requireGroup(gid_t group)
{
  file_group_set = true;
  file_group = group;
}

void econf_requirePermissions(mode_t file_perms, mode_t dir_perms)
{
  file_permissions_set = true;
  file_perms_file = file_perms;
  file_perms_dir = dir_perms;
}

void econf_followSymlinks(bool allow)
{
  allow_follow_symlinks = allow;
}

void econf_reset_security_settings(void)
{
  file_owner_set = false;
  file_group_set = false;
  file_permissions_set = false;
  allow_follow_symlinks = true;
}

econf_err econf_set_conf_dirs(const char **dir_postfix_list)
{
  // free old entry
  if (conf_dirs) econf_freeArray(conf_dirs);
  conf_count = 0;
  const char **tmp = dir_postfix_list;
  while (*tmp++)
    conf_count++;
  conf_dirs = malloc(sizeof(char *) * (conf_count+1));
  if (!conf_dirs)
    return ECONF_NOMEM;
  conf_dirs[conf_count]=NULL;
  for (int i = 0; i < conf_count; i++)
  {
    conf_dirs[i] = strdup(dir_postfix_list[i]);
  }
  return ECONF_SUCCESS;
}

// Create a new econf_file. Allocation is based on
// KEY_FILE_DEFAULT_LENGTH defined in include/defines.h
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

char econf_comment_tag(econf_file *key_file) {
  if (key_file == NULL)
    return '\0';
  return key_file->comment;
}

char econf_delimiter_tag(econf_file *key_file) {
  if (key_file == NULL)
    return '\0';
  return key_file->delimiter;
}

void econf_set_comment_tag(econf_file *key_file, const char comment) {
  if (key_file == NULL)
    return;
  key_file->comment = comment;
}

void econf_set_delimiter_tag(econf_file *key_file, const char delimiter) {
  if (key_file == NULL)
    return;
  key_file->delimiter = delimiter;
}

// Process the file of the given file_name and save its contents into key_file
econf_err econf_readFileWithCallback(econf_file **key_file, const char *file_name,
				     const char *delim, const char *comment,
				     bool (*callback)(const char *filename, const void *data),
				     const void *callback_data)
{
  econf_err t_err;
  struct stat sb;

  if (key_file == NULL || file_name == NULL || delim == NULL || comment == NULL)
    return ECONF_ERROR;

  // Checking file permissions, uid, group,...
  if (lstat(file_name, &sb) == -1)
    return ECONF_NOFILE;
  if (!allow_follow_symlinks && (sb.st_mode&S_IFMT) == S_IFLNK)
    return ECONF_ERROR_FILE_IS_SYM_LINK;
  if (file_owner_set && sb.st_uid != file_owner)
    return ECONF_WRONG_OWNER;
  if (file_group_set && sb.st_gid != file_group)
    return ECONF_WRONG_GROUP;
  if (file_permissions_set) {
    struct stat sb_dir;
    if (!(sb.st_mode&file_perms_file))
      return ECONF_WRONG_FILE_PERMISSION;
    char *cdirc = strdup(file_name);
    int dir_stat = lstat(dirname(cdirc), &sb_dir);
    free(cdirc);
    if ( dir_stat == -1)
      return ECONF_NOFILE;
    if (!(sb_dir.st_mode&file_perms_dir))
      return ECONF_WRONG_DIR_PERMISSION;
  }

  // calling user defined checks
  if (callback != NULL && !(*callback)(file_name, callback_data))
    return ECONF_PARSING_CALLBACK_FAILED;
  
  // Get absolute path if not provided
  char *absolute_path = get_absolute_path(file_name, &t_err);
  if (absolute_path == NULL)
    return t_err;

  *key_file = calloc(1, sizeof(econf_file));
  if (*key_file == NULL) {
    free (absolute_path);
    return ECONF_NOMEM;
  }

  if (*comment)
    (*key_file)->comment = comment[0];
  else {
    (*key_file)->comment = '#';
    comment = "#";
  }

  t_err = read_file(*key_file, absolute_path, delim, comment);
  
  free (absolute_path);

  if(t_err) {
    econf_free(*key_file);
    *key_file = NULL;
    return t_err;
  }

  return ECONF_SUCCESS;
}

econf_err econf_readFile(econf_file **key_file, const char *file_name,
			 const char *delim, const char *comment)
{
   return econf_readFileWithCallback(key_file, file_name, delim, comment, NULL, NULL);
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
  (*merged_file)->path = NULL;
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

econf_err econf_readConfigWithCallback(econf_file **key_file,
				       const char *project,
				       const char *usr_subdir,
				       const char *config_name,
				       const char *config_suffix,
				       const char *delim,
				       const char *comment,
				       bool (*callback)(const char *filename, const void *data),
				       const void *callback_data)
{
  char usr_dir[PATH_MAX];
  char run_dir[PATH_MAX];
  char etc_dir[PATH_MAX];
  econf_err ret = ECONF_SUCCESS;

  if ( config_name == NULL || strlen(config_name) == 0) {
    /* Drop-ins without Main Configuration File. */
    /* e.g. parsing /usr/lib/<project>.d/a.conf, /usr/lib/<project>.d/b.conf and /etc/<project>.d/c.conf */
    /* https://uapi-group.org/specifications/specs/configuration_files_specification/#drop-ins-without-main-configuration-file */
    config_name = project;
    project = NULL;
    const char *dirs[] = {".d", NULL};
    ret = econf_set_conf_dirs(dirs);
    if (ret != ECONF_SUCCESS)
      return ret;
  }

#ifdef TESTSDIR
  if (project != NULL) {
    snprintf(usr_dir, sizeof(usr_dir), "%s%s/%s", TESTSDIR, usr_subdir, project);
    snprintf(run_dir, sizeof(run_dir), "%s%s/%s", TESTSDIR, DEFAULT_RUN_SUBDIR, project);
    snprintf(etc_dir, sizeof(etc_dir), "%s%s/%s", TESTSDIR, DEFAULT_ETC_SUBDIR, project);
  } else {
    snprintf(usr_dir, sizeof(usr_dir), "%s%s", TESTSDIR, usr_subdir);
    snprintf(run_dir, sizeof(run_dir), "%s%s", TESTSDIR, DEFAULT_RUN_SUBDIR);
    snprintf(etc_dir, sizeof(etc_dir), "%s%s", TESTSDIR, DEFAULT_ETC_SUBDIR);
  }
#else
  if (project != NULL) {
    snprintf(usr_dir, sizeof(usr_dir), "%s/%s", usr_subdir, project);
    snprintf(run_dir, sizeof(run_dir), "%s/%s", DEFAULT_RUN_SUBDIR, project);
    snprintf(etc_dir, sizeof(etc_dir), "%s/%s", DEFAULT_ETC_SUBDIR, project);
  } else {
    snprintf(usr_dir, sizeof(usr_dir), "%s", usr_subdir);
    snprintf(run_dir, sizeof(run_dir), "%s", DEFAULT_RUN_SUBDIR);
    snprintf(etc_dir, sizeof(etc_dir), "%s", DEFAULT_ETC_SUBDIR);
  }
#endif

  ret = readConfigWithCallback(key_file,
			       usr_dir,
			       run_dir,
			       etc_dir,
			       config_name,
			       config_suffix,
			       delim,
			       comment,
			       conf_dirs,
			       conf_count,
			       callback,
			       callback_data);
  return ret;
}  


econf_err econf_readConfig (econf_file **key_file,
			    const char *project,
			    const char *usr_subdir,
			    const char *config_name,
			    const char *config_suffix,
			    const char *delim,
			    const char *comment)
{
  return econf_readConfigWithCallback(key_file,
				      project,
				      usr_subdir,
				      config_name,
				      config_suffix,
				      delim,
				      comment,
				      NULL,
				      NULL);
}

econf_err econf_readDirsHistoryWithCallback(econf_file ***key_files,
					    size_t *size,
					    const char *dist_conf_dir,
					    const char *etc_conf_dir,
					    const char *config_name,
					    const char *config_suffix,
					    const char *delim,
					    const char *comment,
					    bool (*callback)(const char *filename, const void *data),
					    const void *callback_data)
{
   return readConfigHistoryWithCallback(key_files,
					size,
					dist_conf_dir,
					NULL,
					etc_conf_dir,
					config_name,
					config_suffix,
					delim,
					comment,
					conf_dirs,
					conf_count,
					callback,
					callback_data);
}

econf_err econf_readDirsHistory(econf_file ***key_files,
				size_t *size,
				const char *dist_conf_dir,
				const char *etc_conf_dir,
				const char *config_name,
				const char *config_suffix,
				const char *delim,
				const char *comment) {
  return readConfigHistoryWithCallback(key_files, size, dist_conf_dir, NULL,
				       etc_conf_dir, config_name,
				       config_suffix, delim, comment,
				       conf_dirs, conf_count,
				       NULL, NULL);
}

econf_err econf_readDirsWithCallback(econf_file **result,
				     const char *dist_conf_dir,
				     const char *etc_conf_dir,
				     const char *config_name,
				     const char *config_suffix,
				     const char *delim,
				     const char *comment,
				     bool (*callback)(const char *filename, const void *data),
				     const void *callback_data)
{
  return readConfigWithCallback(result, dist_conf_dir, NULL,
				etc_conf_dir, config_name,
				config_suffix, delim, comment,
				conf_dirs, conf_count,
				callback, callback_data);
}

econf_err econf_readDirs(econf_file **result,
			 const char *dist_conf_dir,
			 const char *etc_conf_dir,
			 const char *config_name,
			 const char *config_suffix,
			 const char *delim,
			 const char *comment)
{
  return readConfigWithCallback(result, dist_conf_dir, NULL,
				etc_conf_dir, config_name,
				config_suffix, delim, comment,
				conf_dirs, conf_count,
				NULL, NULL);
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
  if (kf == NULL) {
    free(save_to);
    return ECONF_WRITEERROR;
  }

  // Write to file
  for (size_t i = 0; i < key_file->length; i++) {
    // Writing group
    if (!i || strcmp(key_file->file_entry[i - 1].group,
                     key_file->file_entry[i].group)) {
      if (i)
        fprintf(kf, "\n");
      if (strcmp(key_file->file_entry[i].group, KEY_FILE_NULL_VALUE)) {
	char *group = addbrackets(key_file->file_entry[i].group);
	fprintf(kf, "%s\n", group);
        free(group);
      }
    }

    // Writing heading comments
    if (key_file->file_entry[i].comment_before_key &&
	strlen(key_file->file_entry[i].comment_before_key) > 0) {
      char buf[BUFSIZ];
      char *line;
      char *value_string = buf;

      strncpy(buf,key_file->file_entry[i].comment_before_key,BUFSIZ-1);
      buf[BUFSIZ-1] = '\0';
      while ((line = strsep(&value_string, "\n")) != NULL) {
	fprintf(kf, "%c%s\n",
		key_file->comment,
		line);
      }
    }

    // Writing values
    fprintf(kf, "%s%c", key_file->file_entry[i].key, key_file->delimiter);
    if (key_file->file_entry[i].value != NULL) {
      if (key_file->file_entry[i].quotes)
	fprintf(kf, "\"%s\"", key_file->file_entry[i].value);
      else
	fprintf(kf, "%s", key_file->file_entry[i].value);
    }

    // Writing rest of comments
    if (key_file->file_entry[i].comment_after_value &&
	strlen(key_file->file_entry[i].comment_after_value) > 0) {
      char buf[BUFSIZ];
      char *line;
      char *value_string = buf;

      strncpy(buf,key_file->file_entry[i].comment_after_value,BUFSIZ-1);
      buf[BUFSIZ-1] = '\0';
      while ((line = strsep(&value_string, "\n")) != NULL) {
	fprintf(kf, " %c%s\n",
		key_file->comment,
		line);
      }
    }
    fprintf(kf, "\n");
  }

  // Clean up
  free(save_to);
  fclose(kf);
  return ECONF_SUCCESS;
}

extern char *econf_getPath(econf_file *kf)
{
  if (kf->path == NULL)
    return strdup("");
  return strdup(kf->path);
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
  if (*groups == NULL) {
    free(uniques);
    return ECONF_NOMEM;
  }

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
  if (length != NULL)
    *length = 0; /* initialize */
  if (!kf)
    return ECONF_ERROR;

  size_t tmp = 0;
  char *group = (!grp || !*grp) ? strdup(KEY_FILE_NULL_VALUE) : strdup(grp);
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
  char *grp = group ? strdup(group) : NULL; \
  econf_err error = find_key(*kf, stripbrackets(grp), key, &num); \
  free(grp); \
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
    return ECONF_FILE_LIST_IS_NULL; \
  if (!key || strlen(key)<= 0)	    \
    return ECONF_EMPTYKEY; \
  char *grp = group ? strdup(group) : NULL; \
  econf_err ret = setKeyValue(set ## TYPE ## ValueNum, kf, stripbrackets(grp), key, VALARG); \
  free(grp); \
  return ret; \
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

  if (key_file->file_entry)
  {
    for (size_t i = 0; i < key_file->alloc_length; i++) {
      if (key_file->file_entry[i].group)
	free(key_file->file_entry[i].group);
      if (key_file->file_entry[i].key)
	free(key_file->file_entry[i].key);
      if (key_file->file_entry[i].value)
	free(key_file->file_entry[i].value);
      if (key_file->file_entry[i].comment_before_key)
	free(key_file->file_entry[i].comment_before_key);
      if (key_file->file_entry[i].comment_after_value)
	free(key_file->file_entry[i].comment_after_value);
    }
    free(key_file->file_entry);
  }

  if (key_file->path)
    free(key_file->path);

  free(key_file);
}
