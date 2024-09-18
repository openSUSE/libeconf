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

#pragma once

#include "keyfile.h"

#include <stddef.h>

/* This file contains the declaration of the functions used by econf_mergeFiles
   to merge the contents of two econf_files.  */


/* Insert the content of "etc_file.file_entry" into "fe" if there is no
   group specified.  */
size_t insert_nogroup(econf_file *dest_kf, struct file_entry **fe, econf_file *ef);

/* Merge contents from existing usr_file groups */
size_t merge_existing_groups(econf_file *dest_kf, struct file_entry **fe, econf_file *uf, econf_file *ef,
                             const size_t etc_start);

/* Add entries from etc_file exclusive groups */
size_t add_new_groups(econf_file *dest_kf, struct file_entry **fe,
		      econf_file *uf, econf_file *ef,
                      const size_t merge_length);

/* Returns the default dirs to iterate through when merging */
char **get_default_dirs(const char *usr_conf_dir, const char *etc_conf_dir);

/* Receives a list of config directories to look for and calls 'check_conf_dir' */
econf_err traverse_conf_dirs(econf_file ***key_files, char *conf_dirs[],
			     size_t *size, const char *path,
			     const char *config_suffix,
			     const char *delim, const char *comment,
			     const bool join_same_entries, const bool python_style,
			     bool (*callback)(const char *filename, const void *data),
			     const void *callback_data);

/* Merge an array of given econf_files into one */
econf_err merge_econf_files(econf_file **key_files, econf_file **merged_files);
