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
 * @file keyfile.h
 * @brief This file contains the definition of the econf_file struct declared in
 * libeconf.h as well as the functions to get and set a specified element
 * of the struct. All functions return an error code != 0 on error.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

/**
 * @brief Definition of the econf_file struct and its inner file_entry struct.
 * @struct econf_file
 * @var econf_file::file_entry
 * If length exceeds alloc_length it is increased.
 *
 * TODO
 * @todo: char delimiter, comment
 *       Should eventually be of type *char to allow multiple character
 *       delimiters and comment indicators. Alternatively they could be
 *       removed from the struct and be passed as parameters to the
 *       read/write functions for config files.
 */
typedef struct econf_file {
   /**
   * @brief Definition of the file_entry struct.
   * @struct file_entry
   *
   * The file_entry struct contains the group, key and value of every
   * key/value entry found in a config file or set via the set functions.
   * If no group is found or provided the group is set to KEY_FILE_NULL_VALUE.
   */
  struct file_entry {
    char *group;           /**< The group of a config file. */
    char *key;             /**< The key inside a group. */
    char *value;           /**< The value of the key. */
    uint64_t line_number;  /**< The line numbers per key. */
  } * file_entry;

  size_t length;           /**< The amount of key/value entries in econf_file. */
  size_t alloc_length;     /**< The amount of allocated file_entry elements.  */
  char delimiter;          /**< The character used to assign a value to a key. */
  char comment;            /**< The character used to specify comments. */
  bool on_merge_delete;    /**< Should econf_file be freed after being merged. */
  char *path;              /**< The path of the config file. */
} econf_file;              /**< typedef */

/**
 * @brief Increase length and alloc_length of key_file by one and initialize
 *        new elements of struct file_entry.
 * @param key_file The econf_file struct.
 * @return An econf_err error code.
 */
econf_err key_file_append(econf_file *key_file);

/**
 * @brief Functions used to get a set value from key_file depending on num.
 * @defgroup getter getter functions
 *
 * @param key_file The econf_file struct to get values from.
 * @param num The respective instance of the file_entry array.
 * @param result A pointer of fitting type to write the result into.
 * TODO
 * @todo: Error checking and defining return value on error needs to done.
 *
 */
/** @ingroup getter
 * @{
 */
econf_err getIntValueNum(econf_file key_file, size_t num, int32_t *result);
econf_err getInt64ValueNum(econf_file key_file, size_t num, int64_t *result);
econf_err getUIntValueNum(econf_file key_file, size_t num, uint32_t *result);
econf_err getUInt64ValueNum(econf_file key_file, size_t num, uint64_t *result);
econf_err getFloatValueNum(econf_file key_file, size_t num, float *result);
econf_err getDoubleValueNum(econf_file key_file, size_t num, double *result);
econf_err getStringValueNum(econf_file key_file, size_t num, char **result);
econf_err getBoolValueNum(econf_file key_file, size_t num, bool *result);
/** @}*/

/**
 * @brief Set the group of the file_entry element with number num.
 * @param key_file The econf_file struct.
 * @param num The element number.
 * @param value The value to set the group to.
 * @return An econf_err error code.
 */
econf_err setGroup(econf_file *key_file, size_t num, const char *value);

/**
 * @brief Set the key of the file_entry element with number num.
 * @param key_file The econf_file struct.
 * @param num The element number.
 * @param value The value to set the key to.
 * @return An econf_err error code.
 */
econf_err setKey(econf_file *key_file, size_t num, const char *value);

/**
 * @brief Functions used to set a value from key_file depending on num.
 * @defgroup setter setter functions
 *
 * @param key_file The econf_file struct to get values from.
 * @param num The respective instance of the file_entry array.
 * @param value A void pointer which is casted to the corresponding fitting
 *        type.
 */

/** @ingroup setter
 * @{
 */
econf_err setIntValueNum(econf_file *key_file, size_t num, const void *value);
econf_err setInt64ValueNum(econf_file *key_file, size_t num, const void *value);
econf_err setUIntValueNum(econf_file *key_file, size_t num, const void *value);
econf_err setUInt64ValueNum(econf_file *key_file, size_t num, const void *value);
econf_err setFloatValueNum(econf_file *key_file, size_t num, const void *value);
econf_err setDoubleValueNum(econf_file *key_file, size_t num, const void *value);
econf_err setStringValueNum(econf_file *key_file, size_t num, const void *value);
econf_err setBoolValueNum(econf_file *key_file, size_t num, const void *value);
/** @}*/
