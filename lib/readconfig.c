#include "libeconf.h"
#include "defines.h"
#include "getfilecontents.h"
#include "readconfig.h"

#include "helpers.h"
#include "keyfile.h"
#include "mergefiles.h"

#include <libgen.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

econf_err readConfigHistoryWithCallback(econf_file ***key_files,
					size_t *size,
					char **parse_dirs,
					const int parse_dirs_count,
					const char *config_name,
					const char *config_suffix,
					const char *delim,
					const char *comment,
					const bool join_same_entries,
					const bool python_style,
					char **conf_dirs,
					const int conf_count,
					bool (*callback)(const char *filename, const void *data),
					const void *callback_data)
{
  const char *suffix = "";
  char *filename, *cp;
  econf_file *key_file = NULL;
  econf_err error = ECONF_SUCCESS;

  *size = 0;

  if (delim == NULL)
    return ECONF_ERROR;

  if (config_name != NULL && strlen (config_name) != 0)
  {
    /* Reading main configuration file. */
    if (config_suffix != NULL && strlen (config_suffix) > 0)
    {
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
    }

    /* Go reverse back of the different parsing directories in order
       to parse the "main" configuration file */
    for (int i = parse_dirs_count; i > 0; i--)
    {
       filename = alloca(strlen (parse_dirs[i-1]) + strlen (config_name) +
			 strlen (suffix) + 2);
       cp = stpcpy (filename, parse_dirs[i-1]);
       *cp++ = '/';
       cp = stpcpy (cp, config_name);
       stpcpy (cp, suffix);
       if (key_file == NULL) {
	 if ((error = econf_newKeyFile_with_options(&key_file, "")) != ECONF_SUCCESS)
           return error;
	 key_file->join_same_entries = join_same_entries;
	 key_file->python_style = python_style;
       }
       error = read_file_with_callback(&key_file, filename, delim, comment,
				       callback, callback_data);
       if (error && error != ECONF_NOFILE) {
	  econf_free(key_file);
	  return error;
       }
       if (error == ECONF_SUCCESS)
       {
	  *size = 1;
          break;
       }
    }
  }

  /* create space to store the econf_files for merging */
  *size = *size+1;
  *key_files = calloc(*size, sizeof(econf_file*));
  if (*key_files == NULL) {
    econf_freeFile(key_file);
    return ECONF_NOMEM;
  }

  if (*size == 2) {
    key_file->on_merge_delete = 1;
    (*key_files)[0] = key_file;
  } else {
    econf_free(key_file);
  }
  (*key_files)[*size-1] = NULL;

  /*
    Indicate which directories to look for. The order is:
    "default_dirs/config_name.suffix.d/"
    AND all other directories which have been set by
    econf_set_conf_dirs. E.G.:
       "default_dirs/config_name/conf.d/"
       "default_dirs/config_name.d/"
       "default_dirs/config_name/"
    */
  char **configure_dirs = malloc(sizeof(char *) * (conf_count + 2));
  if (configure_dirs == NULL)
  {
    free(*key_files);
    *key_files = NULL;
    return ECONF_NOMEM;
  }

  if (conf_count == 0)
  {
    char *suffix_d = malloc (strlen(suffix) + 4); /* + strlen(".d/") */
    if (suffix_d == NULL) {
      free(*key_files);
      *key_files = NULL;
      econf_freeArray(configure_dirs);
      return ECONF_NOMEM;
    }
    cp = stpcpy(suffix_d, suffix);
    stpcpy(cp, ".d");
    configure_dirs[0] = suffix_d;
    configure_dirs[1] = NULL;
  } else {
    for (int i = 0; i < conf_count; i++)
    {
      configure_dirs[i] = strdup(conf_dirs[i]);
    }
    configure_dirs[conf_count] = NULL;
  }

  /* merge all files in e.g. <dist_conf_dir>, <run_conf_dir> and <etc_conf_dir> */
  for (int i = 0; i < parse_dirs_count; i++) {
    char *project_path = combine_strings(parse_dirs[i], config_name, '/');
    error = traverse_conf_dirs(key_files, configure_dirs, size, project_path,
			       suffix, delim, comment, join_same_entries, python_style,
			       callback, callback_data);
    free(project_path);
    if (error != ECONF_SUCCESS)
    {
      for(size_t k = 0; k < *size-1; k++)
      {
	econf_freeFile((*key_files)[k]);
      }
      free(*key_files);
      *key_files = NULL;
      econf_freeArray(configure_dirs);
      return error;
    }
  }

  (*size)--;
  (*key_files)[*size] = NULL;
  econf_freeArray(configure_dirs);
  if (*size <= 0)
  {
    free(*key_files);
    *key_files = NULL;
    return ECONF_NOFILE;
  }

  return ECONF_SUCCESS;
}


econf_err readConfigWithCallback(econf_file **result,
				 const char *config_name,
				 const char *config_suffix,
				 const char *delim,
				 const char *comment,
				 char **conf_dirs,
				 const int conf_count,
				 bool (*callback)(const char *filename, const void *data),
				 const void *callback_data)
{
  size_t size = 0;
  econf_file **key_files = NULL;
  econf_err error = ECONF_SUCCESS;

  if (*result == NULL)
    return ECONF_ARGUMENT_IS_NULL_VALUE;

  if ((*result)->conf_count > 0) {
    /* Setting defined in econf_file have higher priority */
    error = readConfigHistoryWithCallback(&key_files,
					  &size,
					  (*result)->parse_dirs,
					  (*result)->parse_dirs_count,
					  config_name,
					  config_suffix,
					  delim,
					  comment,
					  (*result)->join_same_entries,
					  (*result)->python_style,
					  (*result)->conf_dirs,
					  (*result)->conf_count,
					  callback,
					  callback_data);
  } else {
    error = readConfigHistoryWithCallback(&key_files,
					  &size,
					  (*result)->parse_dirs,
					  (*result)->parse_dirs_count,
					  config_name,
					  config_suffix,
					  delim,
					  comment,
					  (*result)->join_same_entries,
					  (*result)->python_style,
					  conf_dirs,
					  conf_count,
					  callback,
					  callback_data);
  }

  if (error != ECONF_SUCCESS)
    return error;

  if (key_files) {
    econf_free(*result);
    *result = NULL;
    // Merge the list of acquired key_files into merged_file
    error = merge_econf_files(key_files, result);
    free(key_files);
  }

  return error;
}

