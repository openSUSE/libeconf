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

/** @file libeconf.h
 * @brief Public API for the econf library.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief libeconf error codes
 */
enum econf_err {
  /** General purpose success code */
  ECONF_SUCCESS = 0,
  /** Generic Error */
  ECONF_ERROR = 1,
  /** Out of memory */  
  ECONF_NOMEM = 2,
  /** Config file not found */
  ECONF_NOFILE = 3,
  /** Group not found */
  ECONF_NOGROUP = 4,
  /** Key not found */
  ECONF_NOKEY = 5,
  /** Key has empty value */
  ECONF_EMPTYKEY = 6,
  /** Error creating or writing to a file */
  ECONF_WRITEERROR = 7,
  /** General syntax error in input file */
  ECONF_PARSE_ERROR = 8,
  /** Missing closing section bracket */
  ECONF_MISSING_BRACKET = 9,
  /** Missing delimiter */
  ECONF_MISSING_DELIMITER = 10,
  /** Empty section name */
  ECONF_EMPTY_SECTION_NAME = 11,
  /** Text after section */
  ECONF_TEXT_AFTER_SECTION = 12
};

typedef enum econf_err econf_err;

/** @brief Generic macro calls setter function depending on value type.
 *  Use: econf_setValue(econf_file *key_file, char *group, char *key,
 *                      _generic_ value);
 *
 *  Replace _generic_ with one of the supported value types.
 *  Supported Types: int, long, unsigned int, unsigned long, float, double,
 *  string (as *char).
 *  Note: Does not detect "yes", "no", 1 and 0 as boolean type. If you want to
 *  set a bool value use "true" or "false" or use setBoolValue() directly.
 */
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

/** @brief Generic macro to free memory allocated by econf_ functions.
 *  Use: econf_free(_generic_ value);
 *
 *  Replace _generic_ with one of the supported value types.
 *  Supported Types: char** and econf_file*.
 */
#define econf_free(value) (( \
  _Generic((value), \
    econf_file*: econf_freeFile , \
    char**: econf_freeArray)) \
(value))

typedef struct econf_file econf_file;

/** @brief Process the file of the given file_name and save its contents into key_file object.
 *
 * @param result content of parsed file
 * @param file_name absolute path of parsed file
 * @param delim delimiters of key/value e.g. "\t ="
 * @param comment array of characters which define the start of a comment
 * @return econf_err ECONF_SUCCESS or error code
 *
 * Usage:
 * @code
 *   #include "libeconf.h"
 *
 *   econf_file *key_file = NULL;
 *   econf_err error;
 *
 *   error = econf_readFile (&key_file, "/etc/test.conf", "=", "#");
 *
 *   econf_free (key_file);
 * @endcode
 *
 * Default behaviour if entries have the same name in one file: The
 * first hit will be returned. Further entries will be ignored.
 * This can be changed by setting the environment variable 
 * ECONF_JOIN_SAME_ENTRIES. In that case entries with the same name
 * will be joined to one single entry.
 */
extern econf_err econf_readFile(econf_file **result, const char *file_name,
				    const char *delim, const char *comment);

/** @brief Merge the contents of two key_files objects. Entries in etc_file will be
 *         prefered.
 *
 * @param merged_file merged data
 * @param usr_file First data block which has to be merged.
 * @param etc_file Second data block which has to be merged.
 * @return econf_err ECONF_SUCCESS or error code
 *
 *
 * Usage:
 * @code
 *   #include "libeconf.h"
 *
 *   econf_file *key_file_1 = NULL, *key_file_2 = NULL, *key_file_ret = NULL
 *   econf_err error;
 *
 *   error = econf_readFile (&key_file1, "/usr/etc/test.conf", "=", "#");
 *   error = econf_readFile (&key_file2, /etc/test.conf", "=", "#");
 *   error = econf_mergeFiles (&key_file_ret, key_file_1, key_file_2);
 *
 *   econf_free (key_file_ret);
 *   econf_free (key_file_1);
 *   econf_free (key_file_2);
 * @endcode
 *
 */
extern econf_err econf_mergeFiles(econf_file **merged_file,
				       econf_file *usr_file, econf_file *etc_file);

/** @brief Evaluating the content of a given configuration file by reading all needed/available
 *         files in two different directories (normally in /usr/etc and /etc).
 *
 * @param key_file content of parsed file(s)
 * @param usr_conf_dir absolute path of the first directory (normally "/usr/etc")
 * @param etc_conf_dir absolute path of the second directory (normally "/etc")
 * @param project_name basename of the configuration file
 * @param config_suffix suffix of the configuration file. Can also be NULL.
 * @param delim delimiters of key/value e.g. "\t ="
 * @param comment array of characters which define the start of a comment
 * @return econf_err ECONF_SUCCESS or error code
 *
 * Example: Reading content of example.conf in /usr/etc and /etc directory.
 * @code
 *   #include "libeconf.h"
 *
 *   econf_file *key_file = NULL;
 *   econf_err error;
 *
 *   error = econf_readDirs (&key_file,
 *                           "/usr/etc",
 *                           "/etc",
 *                           "example",
 *                           "conf",
 *                           "=", "#");
 *
 *   econf_free (key_file);
 * @endcode
 *
 */
extern econf_err econf_readDirs(econf_file **key_file,
					  const char *usr_conf_dir,
					  const char *etc_conf_dir,
					  const char *project_name,
					  const char *config_suffix,
					  const char *delim,
					  const char *comment);

/* The API/ABI of the following three functions (econf_newKeyFile,
   econf_newIniFile and econf_writeFile) are not stable and will change */

/** @brief Create a new econf_file object.
 *
 * @param result Pointer to the allocated econf_file object.
 * @param delimiter delimiter of key/value e.g. "="
 * @param comment Character which defines the start of a comment.
 * @return econf_err ECONF_SUCCESS or error code
 *
 * Default behaviour if entries have the same name in one file: The
 * first hit will be returned. Further entries will be ignored.
 * This can be changed by setting the environment variable 
 * ECONF_JOIN_SAME_ENTRIES. In that case entries with the same name
 * will be joined to one single entry. 
 */
extern econf_err econf_newKeyFile(econf_file **result, char delimiter, char comment);

/** @brief Create a new econf_file object in IniFile format. So the delimiter
 *         will be "=" and comments are beginning with "#".
 *
 * @param result Pointer to the allocated econf_file object.
 * @return econf_err ECONF_SUCCESS or error code
 *
 */
extern econf_err econf_newIniFile(econf_file **result);


/** @brief Write content of a econf_file struct to specified location.
 *
 * @param key_file Data which has to be written.
 * @param save_to_dir Directory into which the file has to be written.
 * @param file_name filename (with suffix)
 * @return econf_err ECONF_SUCCESS or error code
 *
 */
extern econf_err econf_writeFile(econf_file *key_file, const char *save_to_dir,
				      const char *file_name);

/* --------------- */
/* --- GETTERS --- */
/* --------------- */

/** @brief Evaluating all group entries.
 *
 * @param kf given/parsed data
 * @param length Length of the returned group array.
 * @param groups String array of evaluated groups.
 * @return econf_err ECONF_SUCCESS or error code
 *
 */
extern econf_err econf_getGroups(econf_file *kf, size_t *length, char ***groups);

/** @brief Evaluating all keys.
 *
 * @param kf given/parsed data
 * @param group Group name for which the keys have to be evaluated or
 *        NULL for all keys.
 * @param length Length of the returned key array.
 * @param keys String array of evaluated keys.
 * @return econf_err ECONF_SUCCESS or error code
 *
 */
extern econf_err econf_getKeys(econf_file *kf, const char *group, size_t *length, char ***keys);

/** @brief Evaluating int32 value for given group/key
 *
 * @param kf given/parsed data
 * @param group Desired group or NULL if there is no group defined.
 * @param key Key for which the value is requested.
 * @param result determined value
 * @return econf_err ECONF_SUCCESS or error code
 *
 */
extern econf_err econf_getIntValue(econf_file *kf, const char *group, const char *key, int32_t *result);

/** @brief Evaluating int64 value for given group/key.
 *
 * @param kf given/parsed data
 * @param group Desired group or NULL if there is no group defined.
 * @param key Key for which the value is requested.
 * @param result determined value
 * @return econf_err ECONF_SUCCESS or error code
 *
 */
extern econf_err econf_getInt64Value(econf_file *kf, const char *group, const char *key, int64_t *result);

/** @brief Evaluating uint32 value for given group/key.
 *
 * @param kf given/parsed data
 * @param group Desired group or NULL if there is no group defined.
 * @param key Key for which the value is requested.
 * @param result determined value
 * @return econf_err ECONF_SUCCESS or error code
 *
 */
extern econf_err econf_getUIntValue(econf_file *kf, const char *group, const char *key, uint32_t *result);

/** @brief Evaluating uint64 value for given group/key.
 *
 * @param kf given/parsed data
 * @param group Desired group or NULL if there is no group defined.
 * @param key Key for which the value is requested.
 * @param result determined value
 * @return econf_err ECONF_SUCCESS or error code
 *
 */
extern econf_err econf_getUInt64Value(econf_file *kf, const char *group, const char *key, uint64_t *result);

/** @brief Evaluating float value for given group/key.
 *
 * @param kf given/parsed data
 * @param group Desired group or NULL if there is no group defined.
 * @param key Key for which the value is requested.
 * @param result determined value
 * @return econf_err ECONF_SUCCESS or error code
 *
 */
extern econf_err econf_getFloatValue(econf_file *kf, const char *group, const char *key, float *result);

/** @brief Evaluating double value for given group/key.
 *
 * @param kf given/parsed data
 * @param group Desired group or NULL if there is no group defined.
 * @param key Key for which the value is requested.
 * @param result determined value
 * @return econf_err ECONF_SUCCESS or error code
 *
 */
extern econf_err econf_getDoubleValue(econf_file *kf, const char *group, const char *key, double *result);

/** @brief Evaluating string value for given group/key.
 *
 * @param kf given/parsed data
 * @param group Desired group or NULL if there is no group defined.
 * @param key Key for which the value is requested.
 * @param result A newly allocated string or NULL in error case.
 * @return econf_err ECONF_SUCCESS or error code
 *
 */
extern econf_err econf_getStringValue(econf_file *kf, const char *group, const char *key, char **result);

/** @brief Evaluating bool value for given group/key.
 *
 * @param kf given/parsed data
 * @param group Desired group or NULL if there is no group defined.
 * @param key Key for which the value is requested.
 * @param result determined value
 * @return econf_err ECONF_SUCCESS or error code
 *
 */
extern econf_err econf_getBoolValue(econf_file *kf, const char *group, const char *key, bool *result);

/** @brief Evaluating int32 value for given group/key.
 *         If key is not found, the default value is returned and error is ECONF_NOKEY.
 *
 * @param kf given/parsed data
 * @param group Desired group or NULL if there is no group defined.
 * @param key Key for which the value is requested.
 * @param result determined value
 * @param def Default value if the value has not been found.
 * @return econf_err ECONF_SUCCESS or error code
 *
 */
extern econf_err econf_getIntValueDef(econf_file *kf, const char *group, const char *key, int32_t *result, int32_t def);

/** @brief Evaluating int64 value for given group/key.
 *         If key is not found, the default value is returned and error is ECONF_NOKEY.
 *
 * @param kf given/parsed data
 * @param group Desired group or NULL if there is no group defined.
 * @param key Key for which the value is requested.
 * @param result determined value
 * @param def Default value if the value has not been found.
 * @return econf_err ECONF_SUCCESS or error code
 *
 */
extern econf_err econf_getInt64ValueDef(econf_file *kf, const char *group, const char *key, int64_t *result, int64_t def);

/** @brief Evaluating uint32 value for given group/key.
 *         If key is not found, the default value is returned and error is ECONF_NOKEY.
 *
 * @param kf given/parsed data
 * @param group Desired group or NULL if there is no group defined.
 * @param key Key for which the value is requested.
 * @param result determined value
 * @param def Default value if the value has not been found.
 * @return econf_err ECONF_SUCCESS or error code
 *
 */
extern econf_err econf_getUIntValueDef(econf_file *kf, const char *group, const char *key, uint32_t *result, uint32_t def);

/** @brief Evaluating uint64 value for given group/key.
 *         If key is not found, the default value is returned and error is ECONF_NOKEY.
 *
 * @param kf given/parsed data
 * @param group Desired group or NULL if there is no group defined.
 * @param key Key for which the value is requested.
 * @param result determined value
 * @param def Default value if the value has not been found.
 * @return econf_err ECONF_SUCCESS or error code
 *
 */
extern econf_err econf_getUInt64ValueDef(econf_file *kf, const char *group, const char *key, uint64_t *result, uint64_t def);

/** @brief Evaluating float value for given group/key.
 *         If key is not found, the default value is returned and error is ECONF_NOKEY.
 *
 * @param kf given/parsed data
 * @param group Desired group or NULL if there is no group defined.
 * @param key Key for which the value is requested.
 * @param result determined value
 * @param def Default value if the value has not been found.
 * @return econf_err ECONF_SUCCESS or error code
 *
 */
extern econf_err econf_getFloatValueDef(econf_file *kf, const char *group, const char *key, float *result, float def);

/** @brief Evaluating double value for given group/key.
 *         If key is not found, the default value is returned and error is ECONF_NOKEY.
 *
 * @param kf given/parsed data
 * @param group Desired group or NULL if there is no group defined.
 * @param key Key for which the value is requested.
 * @param result determined value
 * @param def Default value if the value has not been found.
 * @return econf_err ECONF_SUCCESS or error code
 *
 */
extern econf_err econf_getDoubleValueDef(econf_file *kf, const char *group, const char *key, double *result, double def);

/** @brief Evaluating string value for given group/key.
 *         If key is not found, the default value is returned and error is ECONF_NOKEY.
 *
 * @param kf given/parsed data
 * @param group Desired group or NULL if there is no group defined.
 * @param key Key for which the value is requested.
 * @param result Returns a newly allocated string, even if "default" is returned.
 * @param def Default value if the value has not been found.
 * @return econf_err ECONF_SUCCESS or error code
 *
 */
extern econf_err econf_getStringValueDef(econf_file *kf, const char *group, const char *key, char **result, char *def);

/** @brief Evaluating bool value for given group/key.
 *         If key is not found, the default value is returned and error is ECONF_NOKEY.
 *
 * @param kf given/parsed data
 * @param group Desired group or NULL if there is no group defined.
 * @param key Key for which the value is requested.
 * @param result determined value
 * @param def Default value if the value has not been found.
 * @return econf_err ECONF_SUCCESS or error code
 *
 */
extern econf_err econf_getBoolValueDef(econf_file *kf, const char *group, const char *key, bool *result, bool def);

/* --------------- */
/* --- SETTERS --- */
/* --------------- */

/** @brief Set int32 value for given group/key.
 *
 * @param kf given/parsed data
 * @param group Desired group or NULL if there is no group defined.
 * @param key Key for which the value has to be set.
 * @param value Value which has to be set.
 * @return econf_err ECONF_SUCCESS or error code
 *
 */
extern econf_err econf_setIntValue(econf_file *kf, const char *group, const char *key, int32_t value);

/** @brief Set int64 value for given group/key.
 *
 * @param kf given/parsed data
 * @param group Desired group or NULL if there is no group defined.
 * @param key Key for which the value has to be set.
 * @param value Value which has to be set.
 * @return econf_err ECONF_SUCCESS or error code
 *
 */
extern econf_err econf_setInt64Value(econf_file *kf, const char *group, const char *key, int64_t value);

/** @brief Set uint32 value for given group/key.
 *
 * @param kf given/parsed data
 * @param group Desired group or NULL if there is no group defined.
 * @param key Key for which the value has to be set.
 * @param value Value which has to be set.
 * @return econf_err ECONF_SUCCESS or error code
 *
 */
extern econf_err econf_setUIntValue(econf_file *kf, const char *group, const char *key, uint32_t value);

/** @brief Set uint64 value for given group/key.
 *
 * @param kf given/parsed data
 * @param group Desired group or NULL if there is no group defined.
 * @param key Key for which the value has to be set.
 * @param value Value which has to be set.
 * @return econf_err ECONF_SUCCESS or error code
 *
 */
extern econf_err econf_setUInt64Value(econf_file *kf, const char *group, const char *key, uint64_t value);

/** @brief Set float value for given group/key.
 *
 * @param kf given/parsed data
 * @param group Desired group or NULL if there is no group defined.
 * @param key Key for which the value has to be set.
 * @param value Value which has to be set.
 * @return econf_err ECONF_SUCCESS or error code
 *
 */
extern econf_err econf_setFloatValue(econf_file *kf, const char *group, const char *key, float value);

/** @brief Set double value for given group/key.
 *
 * @param kf given/parsed data
 * @param group Desired group or NULL if there is no group defined.
 * @param key Key for which the value has to be set.
 * @param value Value which has to be set.
 * @return econf_err ECONF_SUCCESS or error code
 *
 */
extern econf_err econf_setDoubleValue(econf_file *kf, const char *group, const char *key, double value);

/** @brief Set string value for given group/key.
 *
 * @param kf given/parsed data
 * @param group Desired group or NULL if there is no group defined.
 * @param key Key for which the value has to be set.
 * @param value Value which has to be set.
 * @return econf_err ECONF_SUCCESS or error code
 *
 */
extern econf_err econf_setStringValue(econf_file *kf, const char *group, const char *key, const char *value);

/** @brief Set bool value for given group/key.
 *
 * @param kf given/parsed data
 * @param group Desired group or NULL if there is no group defined.
 * @param key Key for which the value has to be set.
 * @param value Value which has to be set.
 * @return econf_err ECONF_SUCCESS or error code
 *
 */
extern econf_err econf_setBoolValue(econf_file *kf, const char *group, const char *key, const char *value);

/* --------------- */
/* --- HELPERS --- */
/* --------------- */

/** @brief Convert an econf_err type to a string.
 *
 * @param error error enum
 * @return human readable string
 *
 */
extern const char *econf_errString (const econf_err error);

/** @brief Info about where the error has happened.
 *
 * @param filename Path of the last scanned file.
 * @param line_nr Number of the last handled line.
 *
 */
extern void econf_errLocation (char **filename, uint64_t *line_nr);

/** @brief Free an array of type char** created by econf_getGroups() or econf_getKeys().
 *
 * @param array array of strings
 * @return void
 *
 */
extern void econf_freeArray(char **array);

/** @brief Free memory allocated by e.g. econf_readFile(), econf_readDirs(),...
 *
 * @param key_file allocated data
 * @return void
 *
 */
extern void econf_freeFile(econf_file *key_file);

#ifdef __cplusplus
}
#endif
