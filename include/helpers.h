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
 * @file helpers.h
 * @brief Helper functions to perform certain basic tasks on multiple instances.
 */

#pragma once

#include "keyfile.h"

/**
 * @brief Combine two strings with given delimiter.
 * @param string_one The first string.
 * @param string_two The second string.
 * @param delimiter The delimiter to use.
 * @return The combined string.
 */
char *combine_strings(const char *string_one, const char *string_two,
                      const char delimiter);

/**
 * @brief Remove whitespace from the beginning and the end of a string.
 * @param vlen TODO: The length of the string.
 * @param string The string to remove whitespace from.
 * @return A string with the string terminator appended at the end.
 */
char *clearblank(size_t *vlen, char *string);

/**
 * @brief Return the absolute path of a given path.
 * @param path The given path.
 * @param error Econf_err pointer for storing an error code.
 * @return The absolut path.
 */
char *get_absolute_path(const char *path, econf_err *error);

/**
 * @brief Remove '[' and ']' brackets from the beginning and the end of a string.
 * @param string The string to remove brackets from.
 * @return The string without brackets.
 *
 * TODO
 * @todo: The function is currently not in use. But needs to be used to strip
 *        the brackets from group elements before returning them to the user.
 */
char *stripbrackets(char *string);

/**
 * @brief Add '[' and ']' brackets to the given string.
 * @param string The string to add brackets.
 * @return The string with brackets.
 */
char *addbrackets(const char *string);

/**
 * @brief Set default values defined in defines.h.
 * @param key_file The econf_file struct.
 * @param num The number of the element in the econf_file struct.
 * @return void.
 */
void initialize(econf_file *key_file, size_t num);

/**
 * @brief Return the lower case version of a string.
 * @param str The string to make lower case.
 * @return The lower case version of a string.
 */
char *toLowerCase(char *str);

/**
 * @brief Hashes the given string with djb2.
 * @param str The string to hash.
 * @return A djb2 hash of the string.
 */
size_t hashstring(const char *str);

/**
 * @brief Look for a matching key in the given econf_file.
 * @param key_file The econf_file struct in which to look for the key.
 * @param group A group where the key is located (optional).
 * @param key The key to look for.
 * @param num A pointer which points to the array index of the key
 *                    or -1.
 * @return An econf_err error code. If the key is found num will
 *         point to the array index of the key or be -1.
 */
econf_err find_key(econf_file key_file, const char *group, const char *key, size_t *num);

/**
 * @brief Set values for the given group, key combination.
 * @param function TODO
 * @param kf The econf_file struct.
 * @param group The group of the key.
 * @param key The key to set the value for.
 * @param value The value to se.
 * @return An econf_err error code. If the group - key combination does not
 *         exist, it will be created.
 */
econf_err setKeyValue(econf_err (*function) (econf_file*, size_t, const void*),
                      econf_file *kf, const char *group, const char *key,
                      const void *value);

/**
 * @brief Copy the contents of a file_entry struct.
 * @param fe The file_entry struct to copy.
 * @return A copy of the file_entry fe.
 */
struct file_entry cpy_file_entry(struct file_entry fe);
