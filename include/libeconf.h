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
 * @file libeconf.h
 * @brief The public API for the econf library.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/**
 * @brief The error codes in libeconf.
 * @enum econf_err
 */
enum econf_err {
  ECONF_SUCCESS = 0,    /**< 0: General purpose success code. */
  ECONF_ERROR = 1,      /**< 1: Generic error. */
  ECONF_NOMEM = 2,      /**< 2: Out of memory. */
  ECONF_NOFILE = 3,     /**< 3: Config file not found. */
  ECONF_NOGROUP = 4,    /**< 4: Group not found. */
  ECONF_NOKEY = 5,      /**< 5: Key not found. */
  ECONF_EMPTYKEY = 6,   /**< 6: Key has empty value. */
  ECONF_WRITEERROR = 7, /**< 7: Error creating or writing to a file. */
  ECONF_PARSE_ERROR = 8 /**< 8: Syntax error in input file. */
};

/**
 * @typedef econf_err
 */
typedef enum econf_err econf_err; /**< see @ref econf_err */

/**
 * @struct econf_file
 */
typedef struct econf_file econf_file; /**< econf_file struct, see keyfile.h */

/**
 * @brief Generic macro calls @ref setter depending on the value type.
 * @def econf_setValue(kf, group, key, value)
 * @param key_file The econf_file struct.
 * @param group The group in which the key is present.
 * @param key The key to modify.
 * @param value The value to set.
 *
 * @note Usage:
 * @code
 *      econf_setValue(econf_file *key_file, char *group, char *key, _generic_ value);
 * @endcode
 * Replace _generic_ with one of the supported value types.
 * Supported Types: int, long, unsigned int, unsigned long, float, double,
 *                  string (as *char).
 *
 * @warning Does not detect "yes", "no", 1 and 0 as boolean type. If you want to
 *       set a bool value use "true" or "false" or use setBoolValue() directly.
 */
#define econf_setValue(key_file, group, key, value) ((         \
    _Generic((value),                                          \
    int: econf_setIntValue,                                    \
    long: econf_setInt64Value,                                 \
    unsigned int: econf_setUIntValue,                          \
    unsigned long: econf_setUInt64Value,                       \
    float: econf_setFloatValue,                                \
    double: econf_setDoubleValue,                              \
    char*: econf_setStringValue, void*: econf_setStringValue)) \
(key_file, group, key, value))



/**
 * @brief Generic macro to free memory allocated by econf_ functions.
 * @note Usage:
 * @code
 *      econf_free(_generic_ value);
 * @endcode
 * Replace _generic_ with one of the supported value types.
 * Supported Types: char** and econf_file*.
 */
#define econf_free(value) ((      \
    _Generic((value),             \
    econf_file*: econf_freeFile , \
    char**: econf_freeArray))     \
(value))


/**
 * @brief Process the file of the given file_name and save its contents into a key_file.
 * @param result The key file where the content is saved.
 * @param file_name The file name of the file to process.
 * @param delimiter The delimiter used in the file.
 * @param comment The character used for comments.
 * @result An econf_err error code.
 */
extern econf_err econf_readFile(econf_file **result, const char *file_name,
				const char *delimiter, const char *comment);

/**
 * @brief Merge the contents of two key files.
 * @param merged_file The resulting merge file.
 * @param usr_file The /usr file to merge.
 * @param etc_file The /etc file to merge.
 * @result An econf_err error code.
 */
extern econf_err econf_mergeFiles(econf_file **merged_file,
				  econf_file *usr_file, econf_file *etc_file);

/**
 * @brief Read the specified config file in /etc and /usr.
 * @param key_file The econf_file struct.
 * @param usr_conf_dir The /usr path of the config file.
 * @param etc_conf_dir The /etc path of the config file.
 * @param project_name The file name itself.
 * @param config_suffix The file extension.
 * @param delimiter The delimiter used in the config file.
 * @param comment The comment character used in the config file.
 * @result An econf_err error code.
 */
extern econf_err econf_readDirs(econf_file **key_file,
				const char *usr_conf_dir,
				const char *etc_conf_dir,
				const char *project_name,
				const char *config_suffix,
				const char *delimiter,
				const char *comment);

/**
 * @brief Create a new econf_file key file.
 * @param result The new econf_file file.
 * @param delimiter The delimiter used in the config file.
 * @param comment The comment character used in the config file.
 * @result An econf_err error code.
 *
 * @warning The API/ABI is not stable and will change!
 */
extern econf_err econf_newKeyFile(econf_file **result, char delimiter, char comment);

/**
 * @brief Create a new econf_file ini file. Basically calls econf_newKeyFile().
 * @param result The new econf_file file.
 * @result An econf_err error code.
 *
 * @warning The API/ABI is not stable and will change!
 */
extern econf_err econf_newIniFile(econf_file **result);

/**
 * @brief Write content of an econf_file struct to a specified location.
 * @param key_file The econf_file struct.
 * @param save_to_dir The directory where the file will be saved.
 * @param file_name The name of the file.
 * @result An econf_err error code.
 *
 * @warning The API/ABI is not stable and will change!
 */
extern econf_err econf_writeFile(econf_file *key_file, const char *save_to_dir,
				 const char *file_name);


/**
 * @brief Functions used to get data from an econf_file key_file.
 * @defgroup econf_getter econf getter functions
 */

/**
 * @brief Retrieve the groups of a specified key_file.
 * @ingroup econf_getter
 *
 * @param kf The econf_file struct.
 * @param length
 * @param groups The groups to retrieve.
 * @return An econf_err error code.
 */
extern econf_err econf_getGroups(econf_file *kf, size_t *length, char ***groups);

/**
 * @brief Retrieve the keys in group of a specified key_file.
 * @ingroup econf_getter
 *
 * @param kf The econf_file struct.
 * @param group The group the key belongs to.
 * @param length
 * @param keys The keys to retrieve.
 * @return An econf_err error code.
 */
extern econf_err econf_getKeys(econf_file *kf, const char *group, size_t *length, char ***keys);

/**
 * @brief Functions used to get a set value from key_file.
 * @ingroup econf_getter
 *
 * @param kf The econf_file struct.
 * @param group The group the key belongs to.
 * @param key The key to retrieve.
 * @param result Contains the result to be get.
 * @return An econf_err error code.
 */
/** @ingroup econf_getter
 * @{
 */
extern econf_err econf_getIntValue(econf_file *kf, const char *group, const char *key, int32_t *result);
extern econf_err econf_getInt64Value(econf_file *kf, const char *group, const char *key, int64_t *result);
extern econf_err econf_getUIntValue(econf_file *kf, const char *group, const char *key, uint32_t *result);
extern econf_err econf_getUInt64Value(econf_file *kf, const char *group, const char *key, uint64_t *result);
extern econf_err econf_getFloatValue(econf_file *kf, const char *group, const char *key, float *result);
extern econf_err econf_getDoubleValue(econf_file *kf, const char *group, const char *key, double *result);
/** @}*/

/**
 * @brief Retrieve the string of a key.
 * @ingroup econf_getter
 *
 * @param kf The econf_file struct.
 * @param group The group the key belongs to.
 * @param key The key to retrieve.
 * @param result Contains the string value or NULL in error case.
 * @return An econf_err error code.
 */
extern econf_err econf_getStringValue(econf_file *kf, const char *group, const char *key, char **result);

/**
 * @brief Retrieve the bool value of a key.
 * @ingroup econf_getter
 *
 * @param kf The econf_file struct.
 * @param group The group the key belongs to.
 * @param key The key to retrieve.
 * @param result Contains the bool value or NULL in error case.
 * @return An econf_err error code.
 */
extern econf_err econf_getBoolValue(econf_file *kf, const char *group, const char *key, bool *result);

/**
 * @brief Functions used to get a set value from key_file.
 * @defgroup econf_def_getter econf default getter functions
 *
 * @param kf The econf_file struct.
 * @param group The group the key belongs to.
 * @param key The key to retrieve.
 * @param result Contains the value of the key or the default value if the key is not found.
 * @param def The default value.
 * @return An econf_err error code.
 *
 * @note If key is not found, the default value is returned and the error is ECONF_NOKEY
 */
/** @ingroup econf_def_getter
 * @{
 */
extern econf_err econf_getIntValueDef(econf_file *kf, const char *group, const char *key, int32_t *result, int32_t def);
extern econf_err econf_getInt64ValueDef(econf_file *kf, const char *group, const char *key, int64_t *result, int64_t def);
extern econf_err econf_getUIntValueDef(econf_file *kf, const char *group, const char *key, uint32_t *result, uint32_t def);
extern econf_err econf_getUInt64ValueDef(econf_file *kf, const char *group, const char *key, uint64_t *result, uint64_t def);
extern econf_err econf_getFloatValueDef(econf_file *kf, const char *group, const char *key, float *result, float def);
extern econf_err econf_getDoubleValueDef(econf_file *kf, const char *group, const char *key, double *result, double def);
/** @}*/


/**
 * @brief Retrieves the string of a key.
 * @ingroup econf_def_getter
 *
 * @param kf The econf_file struct.
 * @param group The group the key belongs to.
 * @param key The key to retrieve.
 * @param result Contains the string value even if "default" is returned.
 * @param def The default value to set.
 * @return An econf_err error code.
 */
extern econf_err econf_getStringValueDef(econf_file *kf, const char *group, const char *key, char **result, char *def);

/**
 * @brief Retrieves the bool value of a key.
 * @ingroup econf_def_getter
 *
 * @param kf The econf_file struct.
 * @param group The group the key belongs to.
 * @param key The key to retrieve.
 * @param result Contains the bool value even if "default" is returned.
 * @param def The default value to set.
 * @return An econf_err error code.
 */
extern econf_err econf_getBoolValueDef(econf_file *kf, const char *group, const char *key, bool *result, bool def);


/**
 * @brief Functions used to modify data from an econf_file key_file.
 * @defgroup econf_setter econf setter functions
 *
 * @param kf The econf_file struct.
 * @param group The group the key belongs to
 * @param key The key whose value is to be set.
 * @param value The value to set.
 * @return An econf_err error code.
 */
/** @ingroup econf_setter
 * @{
 */
extern econf_err econf_setIntValue(econf_file *kf, const char *group, const char *key, int32_t value);
extern econf_err econf_setInt64Value(econf_file *kf, const char *group, const char *key, int64_t value);
extern econf_err econf_setUIntValue(econf_file *kf, const char *group, const char *key, uint32_t value);
extern econf_err econf_setUInt64Value(econf_file *kf, const char *group, const char *key, uint64_t value);
extern econf_err econf_setFloatValue(econf_file *kf, const char *group, const char *key, float value);
extern econf_err econf_setDoubleValue(econf_file *kf, const char *group, const char *key, double value);
extern econf_err econf_setStringValue(econf_file *kf, const char *group, const char *key, const char *value);
extern econf_err econf_setBoolValue(econf_file *kf, const char *group, const char *key, const char *value);
/** @}*/


/**
 * @brief Helper functions used in libeconf.
 * @defgroup econf_helper econf helper functions
 */

/**
 * @brief Convert an econf_err type to a string.
 * @ingroup econf_helper
 *
 * @param econf_err The error to convert.
 * @result The resulting error as string.
 */
extern const char *econf_errString (const econf_err);

/**
 * @brief Free an array of type char** created by econf_getGroups() or econf_getKeys().
 * @ingroup econf_helper
 *
 * @param array The array to free.
 * @result void.
 */
extern void econf_freeArray(char **array);

/**
 * @brief Free memory allocated by key_file.
 * @ingroup econf_helper
 *
 * @param key_file The key_file struct to free.
 * @result void.
 */
extern void econf_freeFile(econf_file *key_file);
