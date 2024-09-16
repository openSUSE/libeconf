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

/* -- helpers.h --- */

#include "keyfile.h"

/* This file contains the declaration of functions used by other functions to
   perform certain basic tasks on multiple instances.  */


/* Combine the two given strings with the given delimiter and return the
   resulting string.  */
char *combine_strings(const char *string_one, const char *string_two,
                      const char delimiter);

/* Remove whitespace from beginning and end, append string terminator */
char *clearblank(size_t *vlen, char *string);

/* Returns the absolute path of a given path */
char *get_absolute_path(const char *path, econf_err *error);

/* Remove '[' and ']' from beginning and end */
char *stripbrackets(char *string);

/* Add '[' and ']' to the given string */
char *addbrackets(const char *string);

/* Set default value defined in include/defines.h */
void initialize(econf_file *key_file, size_t num);

/* Return the lower case version of a string */
char *toLowerCase(char *str);

/* Turn given string into a hash value */
size_t hashstring(const char *str);

/* Look for a matching key in the given econf_file.
   If the key is found num will point to the number of the array which contains
   the key, if not it will point to -1.  */
econf_err find_key(econf_file key_file, const char *group, const char *key, size_t *num);

/* Set value for the given group, key combination. If the combination
   does not exist it is created.  */
econf_err setKeyValue(econf_err (*function) (econf_file*, size_t, const void*),
                 econf_file *kf, const char *group, const char *key,
                 const void *value);

/* Copy the contents of a file_entry struct */
/* dest_kf contains the list of group names */
struct file_entry cpy_file_entry(econf_file *dest_kf, struct file_entry fe);

/* Handle groups in an string array */
char *getFromGroupList(econf_file *key_file, const char *name);
char *setGroupList(econf_file *key_file, const char *name);
