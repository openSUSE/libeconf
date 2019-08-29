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
#include "../include/defines.h"
#include "../include/helpers.h"
#include "../include/mergefiles.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Insert the content of "etc_file.file_entry" into "fe" if there is no
// group specified
size_t insert_nogroup(struct file_entry **fe, econf_file *ef) {
  size_t etc_start = 0;
  while (etc_start < ef->length &&
         !strcmp(ef->file_entry[etc_start].group, KEY_FILE_NULL_VALUE)) {
    (*fe)[etc_start] = cpy_file_entry(ef->file_entry[etc_start]);
    etc_start++;
  }
  return etc_start;
}

// Merge contents from existing usr_file groups
// uf: usr_file, ef: etc_file
size_t merge_existing_groups(struct file_entry **fe, econf_file *uf, econf_file *ef,
                             const size_t etc_start) {
  char new_key;
  size_t merge_length = etc_start, tmp = etc_start, added_keys = etc_start;
  for (size_t i = 0; i <= uf->length; i++) {
    // Check if the group has changed in the last iteration
    if (i == uf->length ||
        (i && strcmp(uf->file_entry[i].group, uf->file_entry[i - 1].group))) {
      for (size_t j = etc_start; j < ef->length; j++) {
        // Check for matching groups
        if (!strcmp(uf->file_entry[i - 1].group, ef->file_entry[j].group)) {
          new_key = 1;
          for (size_t k = merge_length; k < i + tmp; k++) {
            // If an existing key is found in ef take the value from ef
            if (!strcmp((*fe)[k].key, ef->file_entry[j].key)) {
              free((*fe)[k].value);
              (*fe)[k].value = strdup(ef->file_entry[j].value);
              new_key = 0;
              break;
            }
          }
          // If a new key is found for an existing group append it to the group
          if (new_key)
            (*fe)[i + added_keys++] = cpy_file_entry(ef->file_entry[j]);
        }
      }
      merge_length = i + added_keys;
      // Temporary value to reduce amount of iterations in inner for loop
      tmp = added_keys;
    }
    if (i != uf->length)
      (*fe)[i + added_keys] = cpy_file_entry(uf->file_entry[i]);
  }
  return merge_length;
}

// Add entries from etc_file exclusive groups
size_t add_new_groups(struct file_entry **fe, econf_file *uf, econf_file *ef,
                      const size_t merge_length) {
  size_t added_keys = merge_length;
  char new_key;
  for (size_t i = 0; i < ef->length; i++) {
    if (!strcmp(ef->file_entry[i].group, KEY_FILE_NULL_VALUE))
      continue;
    new_key = 1;
    for (size_t j = 0; j < uf->length; j++) {
      if (!strcmp(uf->file_entry[j].group, ef->file_entry[i].group)) {
        new_key = 0;
        break;
      }
    }
    if (new_key)
      (*fe)[added_keys++] = cpy_file_entry(ef->file_entry[i]);
  }
  *fe = realloc(*fe, added_keys * sizeof(struct file_entry));
  return added_keys;
}


// TODO: Make this function configureable with econf_set_opt()
char **get_default_dirs(const char *usr_conf_dir, const char *etc_conf_dir) {
  size_t default_dir_number = 3, dir_num = 0;

  char **default_dirs = malloc(default_dir_number * sizeof(char *));
  if (default_dirs == NULL)
    return NULL;

  if (etc_conf_dir)
    {
      // Set config directory in /etc
      default_dirs[dir_num++] = strdup(etc_conf_dir); /* XXX ENOMEM check */
    }
  if (usr_conf_dir)
    {
      // Set config directory in /usr
      default_dirs[dir_num++] = strdup(usr_conf_dir); /* XXX ENOMEM check */
    }
  #if 0
  // TODO: Use secure_getenv() instead and check if the variable is actually set
  // For security reasons, this code should only be enabled if the application
  // sets a flag to do so (introduce econf_set_opt() for this.
  // Using user supplied configs can lead to a security problem (e.g. su)

  // If XDG_CONFIG_DIRS is set check it as well
  if(getenv("XDG_CONFIG_DIRS")) {
    default_dirs = realloc(default_dirs, ++default_dir_number * sizeof(char *));
    default_dirs[dir_num++] = strdup(getenv("XDG_CONFIG_DIRS"));
  }
  // XDG config home
  if (getenv("XDG_CONFIG_HOME")) {
    default_dirs[dir_num++] = strdup(getenv("XDG_CONFIG_HOME"));
  } else {
    sprintf(default_dirs[dir_num++] = malloc(strlen("/home//.config") +
            strlen(getenv("USERNAME")) + 1), "/home/%s/.config",
            getenv("USERNAME"));
  }
  #endif

  default_dirs[dir_num] = NULL;

  return default_dirs;
}

// Check if the given directory exists. If so look for config files
// with the given suffix
static econf_file **
check_conf_dir(econf_file **key_files, size_t *size, const char *path,
	       const char *config_suffix, const char *delim, const char *comment)
{
  struct dirent **de;
  int num_dirs = scandir(path, &de, NULL, alphasort);
  if(num_dirs > 0) {
    for (int i = 0; i < num_dirs; i++) {
      size_t lenstr = strlen(de[i]->d_name);
      size_t lensuffix = strlen(config_suffix);
      if (lensuffix < lenstr &&
          strncmp(de[i]->d_name + lenstr - lensuffix, config_suffix, lensuffix) == 0) {
        char *file_path = combine_strings(path, de[i]->d_name, '/');
        econf_file *key_file;
	econf_err error = econf_readFile(&key_file, file_path, delim, comment);
        free(file_path);
        if(!error && key_file) {
          key_file->on_merge_delete = 1;
          key_files[(*size) - 1] = key_file;
          key_files = realloc(key_files, ++(*size) * sizeof(econf_file *));
        }
      }
      free(de[i]);
    }
    free(de);
  }
  return key_files;
}

econf_file **traverse_conf_dirs(econf_file **key_files, const char *config_dirs,
                              size_t *size, const char *path,
			      const char *config_suffix,
                              const char *delim, const char *comment) {
  char *conf_dirs, *dir, c;
  conf_dirs = strdup(config_dirs);
  /* XXX ENOMEM check, is config_dirs != NULL? */
  while ((dir = strrchr(conf_dirs, ' '))) {
    c = *(dir + 1); *dir = 0;
    dir = strrchr(conf_dirs, ' ');
    char *fulldir = combine_strings(path, &*(dir + 1), c);
    /* XXX ENOMEM/NULL pointer check */
    key_files = check_conf_dir(key_files, size,
        fulldir, config_suffix, delim, comment);
    free (fulldir);
    *dir = 0;
  }
  free(conf_dirs);
  return key_files;
}

econf_err merge_econf_files(econf_file **key_files, econf_file **merged_files) {
  if (key_files == NULL || merged_files == NULL)
    return ECONF_ERROR;

  *merged_files = *key_files++;
  if(!merged_files)
    return ECONF_ERROR;

  while(*key_files) {
    econf_err error;
    econf_file *tmp = *merged_files;

    error = econf_mergeFiles(merged_files, *merged_files, *key_files);
    if (error || merged_files == NULL)
      return error;
    (*merged_files)->on_merge_delete = 1;

    if(tmp->on_merge_delete) { econf_free(tmp); }
    if((*key_files)->on_merge_delete) { econf_free(*key_files); }

    key_files++;
  }
  return ECONF_SUCCESS;
}
