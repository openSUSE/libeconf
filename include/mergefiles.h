/*
  Copyright (C) 2019 SUSE LLC
  Author: Pascal Arlt <parlt@suse.com>
  Author: Dominik Gedon <dgedon@suse.com>

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

/**
 * @file mergefiles.h
 * @brief This file contains the declaration of functions used by
          econf_mergeFiles() to merge the contents of two econf_files.
 */

#pragma once
#include "keyfile.h"
#include <stddef.h>

/**
 * @brief Insert the content of "etc_file.file_entry" into "fe" if there is no
 *        group specified.
 * @param fe The destination file_entry.
 * @param ef The source file.
 * @return Returns the position in the econf_file struct.
 */
size_t insert_nogroup(struct file_entry **fe, econf_file *ef);

/**
 * @brief Merge contents from existing usr_file groups.
 * @param fe The file_etry struct.
 * @param uf The /usr file.
 * @param ef The /etc file.
 * @param etc_start
 * @return
 */
size_t merge_existing_groups(struct file_entry **fe,
				econf_file *uf,
				econf_file *ef,
				const size_t etc_start);

/**
 * @brief Add entries from etc_file exclusive groups.
 * @param fe The file_entry struct.
 * @param uf The /usr file.
 * @param ef The /etc file.
 * @param merge_length
 * @return Returns the number of added keys.
 */
size_t add_new_groups(struct file_entry **fe,
				econf_file *uf,
				econf_file *ef,
				const size_t merge_length);

# if 0
/**
 * @brief Returns the default dirs to iterate through when merging.
 * @param usr_conf_dir
 * @param etc_conf_dir
 * @return
 */
char **get_default_dirs(const char *usr_conf_dir, const char *etc_conf_dir);
#endif

/**
 * @brief Receives a list of configuration  directories to look for and calls
          'check_conf_dir()'.
 * @param key_files The econf_file struct.
 * @param conf_dirs[] The provided list of configuration directories.
 * @param size
 * @param path
 * @param config_suffix
 * @param delim The delimiter used in the config file.
 * @param comment The comment character used in the config file.
 * @return An econf_err error message.
 */
econf_file **traverse_conf_dirs(econf_file **key_files,
				const char *conf_dirs[],
				size_t *size,
				const char *path,
				const char *config_suffix,
				const char *delim,
				const char *comment);

/**
 * @brief Merge an array of given econf_files into one.
 * @param key_files The key files to merge.
 * @param merged_files This includes the merged key files.
 * @return An econf_err error message.
 */
econf_err merge_econf_files(econf_file **key_files, econf_file **merged_files);
