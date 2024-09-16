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
#include "helpers.h"
#include "mergefiles.h"
#include "getfilecontents.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>

// Insert the content of "etc_file.file_entry" into "fe" if there is no
// group specified
size_t insert_nogroup(econf_file *dest_kf, struct file_entry **fe,
		      econf_file *ef) {
  size_t etc_start = 0;
  if (ef) {
    while (etc_start < ef->length &&
	   !strcmp(ef->file_entry[etc_start].group, KEY_FILE_NULL_VALUE)) {
      (*fe)[etc_start] = cpy_file_entry(dest_kf, ef->file_entry[etc_start]);
      etc_start++;
    }
  }
  return etc_start;
}

// Merge contents from existing usr_file groups
// uf: usr_file, ef: etc_file
size_t merge_existing_groups(econf_file *dest_kf, struct file_entry **fe, econf_file *uf,
			     econf_file *ef, const size_t etc_start) {
  bool new_key;
  size_t merge_length = etc_start, tmp = etc_start, added_keys = etc_start;
  if (uf && ef) {
    for (size_t i = 0; i <= uf->length; i++) {
      // Check if the group has changed in the last iteration
      if (i == uf->length ||
	  (i && strcmp(uf->file_entry[i].group, uf->file_entry[i - 1].group))) {
	for (size_t j = etc_start; j < ef->length; j++) {
	  // Check for matching groups
	  if (!strcmp(uf->file_entry[i - 1].group, ef->file_entry[j].group)) {
	    new_key = true;
	    for (size_t k = merge_length; k < i + tmp; k++) {
	      // If an existing key is found in ef take the value from ef
	      if (!strcmp((*fe)[k].key, ef->file_entry[j].key)) {
		free((*fe)[k].value);
		(*fe)[k].value = ef->file_entry[j].value ? strdup(ef->file_entry[j].value) : strdup("");
		new_key = false;
		break;
	      }
	    }
	    // If a new key is found for an existing group append it to the group
	    if (new_key)
	      (*fe)[i + added_keys++] = cpy_file_entry(dest_kf, ef->file_entry[j]);
	  }
	}
	merge_length = i + added_keys;
	// Temporary value to reduce amount of iterations in inner for loop
	tmp = added_keys;
      }
      if (i != uf->length)
	(*fe)[i + added_keys] = cpy_file_entry(dest_kf, uf->file_entry[i]);
    }
  }
  return merge_length;
}

// Add entries from etc_file exclusive groups
size_t add_new_groups(econf_file *dest_kf, struct file_entry **fe,
		      econf_file *uf, econf_file *ef,
		      const size_t merge_length) {
  size_t added_keys = merge_length;
  bool new_key;
  if (uf && ef) {
    for (size_t i = 0; i < ef->length; i++) {
      if (!strcmp(ef->file_entry[i].group, KEY_FILE_NULL_VALUE))
	continue;
      new_key = true;
      for (size_t j = 0; j < uf->length; j++) {
	if (!strcmp(uf->file_entry[j].group, ef->file_entry[i].group)) {
	  new_key = false;
	  break;
	}
      }
      if (new_key)
	(*fe)[added_keys++] = cpy_file_entry(dest_kf, ef->file_entry[i]);
    }
    *fe = realloc(*fe, added_keys * sizeof(struct file_entry));
  }
  return added_keys;
}

// Check if the given directory exists. If so look for config files
// with the given suffix
static econf_err
check_conf_dir(econf_file ***key_files, size_t *size, const char *path,
	       const char *config_suffix, const char *delim, const char *comment,
	       const bool join_same_entries, const bool python_style,
	       bool (*callback)(const char *filename, const void *data),
	       const void *callback_data)
{
  struct dirent **de;
  econf_err error;

  int num_dirs = scandir(path, &de, NULL, alphasort);
  if(num_dirs > 0) {
    for (int i = 0; i < num_dirs; i++) {
      size_t lenstr = strlen(de[i]->d_name);
      size_t lensuffix = strlen(config_suffix);
      if (lensuffix < lenstr &&
          strncmp(de[i]->d_name + lenstr - lensuffix, config_suffix, lensuffix) == 0) {
        char *file_path = combine_strings(path, de[i]->d_name, '/');
        econf_file *key_file = NULL;
	if ((error = econf_newKeyFile_with_options(&key_file, "")) != ECONF_SUCCESS)
          return error;
        key_file->join_same_entries = join_same_entries;
        key_file->python_style = python_style;
	error = read_file_with_callback(&key_file, file_path, delim, comment,
					callback, callback_data);
        free(file_path);
        if(!error && key_file) {
          key_file->on_merge_delete = 1;
          (*key_files)[(*size) - 1] = key_file;
          *key_files = realloc(*key_files, ++(*size) * sizeof(econf_file *));
        } else {
	  for (int k = i; k < num_dirs; k++)
	    free(de[k]);
	  free(de);
	  return error;
	}
      }
      free(de[i]);
    }
    free(de);
  }
  return ECONF_SUCCESS;
}

econf_err traverse_conf_dirs(econf_file ***key_files,
			     char *config_dirs[],
			     size_t *size, const char *path,
			     const char *config_suffix,
			     const char *delim, const char *comment,
	                     const bool join_same_entries, const bool python_style,
			     bool (*callback)(const char *filename, const void *data),
			     const void *callback_data) {
  int i;

  if (config_dirs == NULL)
    return ECONF_NOFILE;

  i = 0;
  while (config_dirs[i] != NULL) {
    char *fulldir, *cp;

    if ((fulldir = malloc (strlen(path) + strlen (config_dirs[i]) + 1)) == NULL)
      return ECONF_NOMEM;

    cp = stpcpy (fulldir, path);
    stpcpy (cp, config_dirs[i++]);
    econf_err error = check_conf_dir(key_files, size,
				     fulldir, config_suffix, delim, comment,
	                             join_same_entries, python_style,
				     callback, callback_data);
    free (fulldir);
    if (error)
      return error;
  }
  return ECONF_SUCCESS;
}

econf_err merge_econf_files(econf_file **key_files, econf_file **merged_files) {
  if (*key_files == NULL || merged_files == NULL)
    return ECONF_ERROR;

  *merged_files = *key_files++;
  if(*merged_files == NULL)
    return ECONF_ERROR;

  while(*key_files) {
    econf_err error;
    econf_file *tmp = *merged_files;
    econf_file **double_key_files = key_files+1;
    char * current_file = basename((*key_files)->path);

    /* key_files are already sorted. If there is a file with the same name with
       a higher priority, the current file will be ignored. 
       e.g. /usr/etc/shells.d/tcsh will not be merged if /etc/shells.d/tcsh exists.
    */
    while (*double_key_files) {
      char * compare_file = basename((*double_key_files)->path);
      if (strcmp(current_file, ".") != 0 && strcmp(current_file, "..") &&
	  strcmp(current_file, compare_file) == 0) {
	break;
      }
      double_key_files++;	    
    }
    
    if (*double_key_files == NULL) {
      error = econf_mergeFiles(merged_files, *merged_files, *key_files);
      if (error || *merged_files == NULL)
        return error;
      (*merged_files)->on_merge_delete = 1;
      if(tmp->on_merge_delete) { econf_free(tmp); }      
    }

    if((*key_files)->on_merge_delete) { econf_free(*key_files); }

    key_files++;
  }

  return ECONF_SUCCESS;
}
