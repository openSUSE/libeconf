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

/* libeconf.h */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/* Public API for the econf library */


/* libeconf error codes */
enum econf_err {
  ECONF_SUCCESS = 0, /* General purpose success code */
  ECONF_ERROR = 1, /* Generic Error */
  ECONF_NOMEM = 2, /* Out of memory */
  ECONF_NOFILE = 3, /* Config file not found */
  ECONF_NOGROUP = 4, /* Group not found */
  ECONF_NOKEY = 5, /* Key not found */
  ECONF_EMPTYKEY = 6, /* Key has empty value */
  ECONF_WRITEERROR = 7, /* Error creating or writing to a file */
  ECONF_PARSE_ERROR = 8 /* Syntax error in input file */
};

typedef enum econf_err econf_err;

/* Generic macro calls setter function depending on value type
   Use: econf_setValue(econf_file *key_file, char *group, char *key,
                       _generic_ value);
   Replace _generic_ with one of the supported value types.
   Supported Types: int, long, unsigned int, unsigned long, float, double,
   string (as *char).
   Note: Does not detect "yes", "no", 1 and 0 as boolean type. If you want to
   set a bool value use "true" or "false" or use setBoolValue() directly.  */
#define econf_setValue(kf, group, key, value) (( \
  _Generic((value), \
    int: econf_setIntValue, \
    long: econf_setInt64Value, \
    unsigned int: econf_setUIntValue, \
    unsigned long: econf_setUInt64Value, \
    float: econf_setFloatValue, \
    double: econf_setDoubleValue, \
    char*: econf_setStringValue, void*: econf_setStringValue)) \
(kf, group, key, value))

/* Generic macro to free memory allocated by econf_ functions
   Use: econf_free(_generic_ value);
   Replace _generic_ with one of the supported value types.
   Supported Types: char** and econf_file*.  */
#define econf_free(value) (( \
  _Generic((value), \
    econf_file*: econf_freeFile , \
    char**: econf_freeArray)) \
(value))

typedef struct econf_file econf_file;

// Process the file of the given file_name and save its contents into key_file
extern econf_err econf_readFile(econf_file **result, const char *file_name,
				    const char *delim, const char *comment);

// Merge the contents of two key files
extern econf_err econf_mergeFiles(econf_file **merged_file,
				       econf_file *usr_file, econf_file *etc_file);

extern econf_err econf_readDirs(econf_file **key_file,
					  const char *usr_conf_dir,
					  const char *etc_conf_dir,
					  const char *project_name,
					  const char *config_suffix,
					  const char *delim,
					  const char *comment);

/* The API/ABI of the following three functions (econf_newKeyFile,
   econf_newIniFile and econf_writeFile) are not stable and will change */

/* Create a new econf_file object */
extern econf_err econf_newKeyFile(econf_file **result, char delimiter, char comment);
extern econf_err econf_newIniFile(econf_file **result);

/* Write content of a econf_file struct to specified location */
extern econf_err econf_writeFile(econf_file *key_file, const char *save_to_dir,
				      const char *file_name);

/* --- GETTERS --- */

extern econf_err econf_getGroups(econf_file *kf, size_t *length, char ***groups);
extern econf_err econf_getKeys(econf_file *kf, const char *group, size_t *length, char ***keys);
extern econf_err econf_getIntValue(econf_file *kf, const char *group, const char *key, int32_t *result);
extern econf_err econf_getInt64Value(econf_file *kf, const char *group, const char *key, int64_t *result);
extern econf_err econf_getUIntValue(econf_file *kf, const char *group, const char *key, uint32_t *result);
extern econf_err econf_getUInt64Value(econf_file *kf, const char *group, const char *key, uint64_t *result);
extern econf_err econf_getFloatValue(econf_file *kf, const char *group, const char *key, float *result);
extern econf_err econf_getDoubleValue(econf_file *kf, const char *group, const char *key, double *result);
/* Returns a newly allocated string or NULL in error case. */
extern econf_err econf_getStringValue(econf_file *kf, const char *group, const char *key, char **result);
extern econf_err econf_getBoolValue(econf_file *kf, const char *group, const char *key, bool *result);

/* If key is not found, the default value is returned and error is ECONF_NOKEY */
extern econf_err econf_getIntValueDef(econf_file *kf, const char *group, const char *key, int32_t *result, int32_t def);
extern econf_err econf_getInt64ValueDef(econf_file *kf, const char *group, const char *key, int64_t *result, int64_t def);
extern econf_err econf_getUIntValueDef(econf_file *kf, const char *group, const char *key, uint32_t *result, uint32_t def);
extern econf_err econf_getUInt64ValueDef(econf_file *kf, const char *group, const char *key, uint64_t *result, uint64_t def);
extern econf_err econf_getFloatValueDef(econf_file *kf, const char *group, const char *key, float *result, float def);
extern econf_err econf_getDoubleValueDef(econf_file *kf, const char *group, const char *key, double *result, double def);
/* Returns a newly allocated string, even if "default" is returned. */
extern econf_err econf_getStringValueDef(econf_file *kf, const char *group, const char *key, char **result, char *def);
extern econf_err econf_getBoolValueDef(econf_file *kf, const char *group, const char *key, bool *result, bool def);

/* --- SETTERS --- */

extern econf_err econf_setIntValue(econf_file *kf, const char *group, const char *key, int32_t value);
extern econf_err econf_setInt64Value(econf_file *kf, const char *group, const char *key, int64_t value);
extern econf_err econf_setUIntValue(econf_file *kf, const char *group, const char *key, uint32_t value);
extern econf_err econf_setUInt64Value(econf_file *kf, const char *group, const char *key, uint64_t value);
extern econf_err econf_setFloatValue(econf_file *kf, const char *group, const char *key, float value);
extern econf_err econf_setDoubleValue(econf_file *kf, const char *group, const char *key, double value);
extern econf_err econf_setStringValue(econf_file *kf, const char *group, const char *key, const char *value);
extern econf_err econf_setBoolValue(econf_file *kf, const char *group, const char *key, const char *value);

/* --- HELPERS --- */

/* convert an econf_err type to a string */
extern const char *econf_errString (const econf_err);

// Free an array of type char** created by econf_getGroups() or econf_getKeys()
extern void econf_freeArray(char **array);

// Free memory allocated by key_file
extern void econf_freeFile(econf_file *key_file);
