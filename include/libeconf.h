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

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/* libeconf error codes */
enum econf_err {
  ECONF_SUCCESS = 0, /* General purpose success code */
#define ECONF_SUCCESS ECONF_SUCCESS
  ECONF_ERROR = 1, /* Generic Error */
#define ECONF_ERROR ECONF_ERROR
  ECONF_NOMEM = 2, /* Out of memory */
#define ECONF_NOMEM ECONF_NOMEM
  ECONF_NOFILE = 3, /* Config file not found */
#define ECONF_NOFILE ECONF_NOFILE
  ECONF_NOGROUP = 4, /* Group not found */
#define ECONF_NOGROUP ECONF_NOGROUP
  ECONF_NOKEY = 5, /* Key not found */
#define ECONF_NOKEY ECONF_NOKEY
  ECONF_EMPTYKEY = 6 /* Key has empty vaue */
#define ECONF_EMPTYKEY ECONF_EMPTYKEY
};
typedef enum econf_err econf_err;

// Generic macro calls setter function depending on value type
// Use: econf_setValue(Key_File *key_file, char *group, char *key,
//                     _generic_ value);
// Replace _generic_ with one of the supported value types.
// Supported Types: int, long, unsigned int, unsigned long, float, double,
// string (as *char).
// Note: Does not detect "yes", "no", 1 and 0 as boolean type. If you want to
// set a bool value use "true" or "false" or use setBoolValue() directly.
#define econf_setValue(kf, group, key, value) (( \
  _Generic((value), \
    int: econf_setIntValue, long: econf_setIntValue, \
    unsigned int: econf_setUIntValue, unsigned long: econf_setUIntValue, \
    float: econf_setFloatValue, \
    double: econf_setDoubleValue, \
    char*: econf_setStringValue, void*: econf_setStringValue)) \
(kf, group, key, value))

// Generic macro to free memory allocated by econf_ functions
// Use: econf_destroy(_generic_ value);
// Replace _generic_ with one of the supported value types.
// Supported Types: char** and Key_File*
#define econf_destroy(value) (( \
  _Generic((value), \
    Key_File*: econf_Key_File_destroy , \
    char**: econf_char_a_destroy)) \
(value))

typedef struct Key_File Key_File;

extern Key_File *econf_newKeyFile(char delimiter, char comment, econf_err *);
extern Key_File *econf_newIniFile(econf_err *);

// Process the file of the given file_name and save its contents into key_file
extern Key_File *econf_get_key_file(const char *file_name, char *delim,
				    const char comment, econf_err *);

// Merge the contents of two key files
extern Key_File *econf_merge_key_files(Key_File *usr_file, Key_File *etc_file, 
                                       econf_err *);

// Write content of a Key_File struct to specified location
extern void econf_write_key_file(Key_File *key_file, const char *save_to_dir,
				 const char *file_name);

extern Key_File *econf_get_conf_from_dirs(const char *usr_conf_dir,
					  const char *etch_conf_dir,
					  char *project_name, char *config_suffix,
					  char *delimt, char comment,
					  econf_err *);

/* --- GETTERS --- */

extern char **econf_getGroups(Key_File *kf, size_t *length);
extern char **econf_getKeys(Key_File *kf, const char *group, size_t *length);
extern int32_t econf_getIntValue(Key_File *kf, char *group, char *key);
extern int64_t econf_getInt64Value(Key_File *kf, char *group, char *key);
extern uint32_t econf_getUIntValue(Key_File *kf, char *group, char *key);
extern uint64_t econf_getUInt64Value(Key_File *kf, char *group, char *key);
extern float econf_getFloatValue(Key_File *kf, char *group, char *key);
extern double econf_getDoubleValue(Key_File *kf, char *group, char *key);
extern char *econf_getStringValue(Key_File *kf, char *group, char *key);
extern bool econf_getBoolValue(Key_File *kf, char *group, char *key);

/* --- SETTERS --- */

extern void econf_setIntValue(Key_File *kf, char *group, char *key, int64_t value);
extern void econf_setUIntValue(Key_File *kf, char *group, char *key, uint64_t value);
extern void econf_setFloatValue(Key_File *kf, char *group, char *key, float value);
extern void econf_setDoubleValue(Key_File *kf, char *group, char *key, double value);
extern void econf_setStringValue(Key_File *kf, char *group, char *key, char *value);
extern void econf_setBoolValue(Key_File *kf, char *group, char *key, char *value);

/* --- HELPERS --- */

/* convert an econf_err type to a string */
extern const char *econf_errString (const econf_err);

// Free an array of type char** created by econf_getGroups() or econf_getKeys()
extern void econf_char_a_destroy(char **array);

// Free memory allocated by key_file
extern void econf_Key_File_destroy(Key_File *key_file);
