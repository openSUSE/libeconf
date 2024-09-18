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
#include <sys/types.h>

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
  /** Key is NULL or has empty value */
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
  ECONF_TEXT_AFTER_SECTION = 12,
  /** Parsed file list is NULL */
  ECONF_FILE_LIST_IS_NULL = 13,
  /** Wrong boolean value (1/0 true/false yes/no) */
  ECONF_WRONG_BOOLEAN_VALUE = 14,
  /** Given key has NULL value */
  ECONF_KEY_HAS_NULL_VALUE = 15,
  /** File has wrong owner */
  ECONF_WRONG_OWNER = 16,
  /** File has wrong group */
  ECONF_WRONG_GROUP = 17,
  /** File has wrong file permissions */
  ECONF_WRONG_FILE_PERMISSION = 18,
  /** File has wrong dir permission */
  ECONF_WRONG_DIR_PERMISSION = 19,
  /** File is a sym link which is not permitted */
  ECONF_ERROR_FILE_IS_SYM_LINK = 20,
  /** User defined parsing callback has failed **/
  ECONF_PARSING_CALLBACK_FAILED = 21,
  /** Given argument is NULL */
  ECONF_ARGUMENT_IS_NULL_VALUE = 22,
  /** Given option not found **/
  ECONF_OPTION_NOT_FOUND = 23,
  /** Value cannot be converted **/
  ECONF_VALUE_CONVERSION_ERROR = 24
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
 * @param result content of parsed file.
 * @param file_name absolute path of parsed file
 * @param delim delimiters of key/value e.g. "\t =".
 *        If delim contains space characters AND none space characters,
 *        multiline values are not parseable.
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
 *   key_file = NULL;
 * @endcode
 *
 */
extern econf_err econf_readFile(econf_file **result, const char *file_name,
				const char *delim, const char *comment);


/** @brief Process the file of the given file_name and save its contents into key_file object.
 *  The user defined function will be called in order e.g. to check the correct file permissions.
 *
 * @param result content of parsed file.
 * @param file_name absolute path of parsed file
 * @param delim delimiters of key/value e.g. "\t =".
 *        If delim contains space characters AND none space characters,
 *        multiline values are not parseable.
 * @param comment array of characters which define the start of a comment
 * @param callback function which will be called for the given filename. This user defined function has
 *        the pathname as parameter and returns true if this file can be parsed. If not, the
 *        parsing will be aborted and ECONF_PARSING_CALLBACK_FAILED will be returned.
 * @param callback_data pointer which will be given to the callback function.
 * @return econf_err ECONF_SUCCESS or error code
 *
 * Usage:
 * @code
 *   #include "libeconf.h"
 *
 *   bool checkFile(const char *filename, const void *data) {
 *      - checking code which returns true or false -
 *	return true;
 *   }
 *
 *   econf_file *key_file = NULL;
 *   econf_err error;
 *
 *   error = econf_readFileWithCallback (&key_file, "/etc/test.conf", "=", "#", checkFile, NULL);
 *
 *   econf_free (key_file);
 *   key_file = NULL;
 * @endcode
 *
 * Default behaviour if entries have the same name in one file: The
 * first hit will be returned. Further entries will be ignored.
 * This can be changed by setting the option JOIN_SAME_ENTRIES
 * (see econf_newKeyFile_with_options). In that case entries with
 * the same name will be joined to one single entry.
 */
extern econf_err econf_readFileWithCallback(econf_file **result, const char *file_name,
					    const char *delim, const char *comment,
					    bool (*callback)(const char *filename, const void *data),
					    const void *callback_data);

/** @brief Merge the contents of two key_files objects. Entries in etc_file will be
 *         preferred.
 *         Comment and delimiter tag will be taken from usr_file. This can be changed
 *         by calling the functions econf_set_comment_tag and econf_set_delimiter_tag.
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


/** @brief Evaluating key/values of a given configuration by reading and merging all
 *         needed/available files from different directories. Order is:
 *
 *  /etc/$project/$config_name.$config_suffix does exist:
 *
 *  - /etc/$project/$config_name.$config_suffix
 *  - /run/$project/$config_name.$config_suffix.d/ *.$config_suffix
 *  - $usr_subdir/$project/$config_name.$config_suffix.d/ *.$config_suffix
 *  - /etc/$project/$config_name.$config_suffix.d/ *.$config_suffix
 *
 *  /etc/$project/$config_name.$config_suffix does NOT exist:
 *
 *    /run/$project/$config_name.$config_suffix does exist:
 *
 *    - /run/$project/$config_name.$config_suffix
 *    - /run/$project/$config_name.$config_suffix.d/ *.$config_suffix
 *    - $usr_subdir/$project/$config_name.$config_suffix.d/ *.$config_suffix
 *    - /etc/$project/$config_name.$config_suffix.d/ *.$config_suffix
 *
 *    /run/$project/$config_name.$config_suffix does NOT exist:
 *
 *    - $usr_subdir/$project/$config_name.$config_suffix
 *    - /run/$project/$config_name.$config_suffix.d/ *.$config_suffix
 *    - $usr_subdir/$project/$config_name.$config_suffix.d/ *.$config_suffix
 *    - /etc/$project/$config_name.$config_suffix.d/ *.$config_suffix
 *
 *  No main $config_name.$config_suffix file is defined or must not be parsed:
 *
 *    - $usr_subdir/$project.d/ *.$config_suffix
 *    - /run/$project.d/ *.$config_suffix
 *    - /etc/$project.d/ *.$config_suffix
 *
 * This call fulfills all requirements, defined by the Linux Userspace API (UAPI) Group
 * chapter "Configuration Files Specification".
 * See: https://uapi-group.org/specifications/specs/configuration_files_specification/
 *
 * @param key_file content of parsed file(s). MUST be initialized (e.g. with NULL).
 * @param project name of the project used as subdirectory, can be NULL
 * @param usr_subdir absolute path of the first directory (often "/usr/lib")
 * @param config_name basename of the configuration file.
 *        If it is NULL, drop-ins without a main configuration file will
 *        be parsed only.
 * @param config_suffix suffix of the configuration file. Can also be NULL.
 * @param delim delimiters of key/value e.g. "\t ="
 *        If delim contains space characters AND none space characters,
 *        multiline values are not parseable.
 * @param comment array of characters which define the start of a comment
 * @return econf_err ECONF_SUCCESS or error code
 *
 * Example: Reading content in different cases in following order:
 *
 *  /etc/foo/example.conf does exist:
 *
 *  - /etc/foo/example.conf
 *  - /usr/lib/foo/example.conf.d/ *.conf
 *  - /run/foo/example.conf.d/ *.conf
 *  - /etc/foo/example.conf.d/ *.conf
 *
 *  /etc/foo/example.conf does NOT exist:
 *
 *    /run/foo/example.conf does exist:
 *
 *    - /run/foo/example.conf
 *    - /usr/lib/foo/example.conf.d/ *.conf
 *    - /run/foo/example.conf.d/ *.conf
 *    - /etc/foo/example.conf.d/ *.conf
 *
 *    /run/foo/example.conf does NOT exist:
 *
 *    - /usr/lib/foo/example.conf
 *    - /usr/lib/foo/example.conf.d/ *.conf
 *    - /run/foo/example.conf.d/ *.conf
 *    - /etc/foo/example.conf.d/ *.conf
 *
 * @code
 *   #include "libeconf.h"
 *
 *   econf_file *key_file = NULL;
 *   econf_err error;
 *
 *   error = econf_readConfig (&key_file,
 *                             "foo",
 *                             "/usr/lib",
 *                             "example",
 *                             "conf",
 *                             "=", "#");
 *
 *   econf_free (key_file);
 *   key_file = NULL;
 * @endcode
 *
 * It is also possible to set additional options for e.g. parsing
 * config files which has python style format or defining the order
 * how to parse the configuration files. For more information have a look to
 * econf_newKeyFile_with_options.
 *
 */
extern econf_err econf_readConfig (econf_file **key_file,
	                           const char *project,
                                   const char *usr_subdir,
				   const char *config_name,
				   const char *config_suffix,
				   const char *delim,
				   const char *comment);

/** @brief Evaluating key/values of a given configuration by reading and merging all
 *         needed/available files from different directories. Order is:
 *
 *  /etc/$project/$config_name.$config_suffix does exist:
 *
 *  - /etc/$project/$config_name.$config_suffix
 *  - $usr_subdir/$project/$config_name.$config_suffix.d/ *.$config_suffix
 *  - /run/$project/$config_name.$config_suffix.d/ *.$config_suffix
 *  - /etc/$project/$config_name.$config_suffix.d/ *.$config_suffix
 *
 *  /etc/$project/$config_name.$config_suffix does NOT exist:
 *
 *    /run/$project/$config_name.$config_suffix does exist:
 *
 *    - /run/$project/$config_name.$config_suffix
 *    - $usr_subdir/$project/$config_name.$config_suffix.d/ *.$config_suffix
 *    - /run/$project/$config_name.$config_suffix.d/ *.$config_suffix
 *    - /etc/$project/$config_name.$config_suffix.d/ *.$config_suffix
 *
 *    /run/$project/$config_name.$config_suffix does NOT exist:
 *
 *    - $usr_subdir/$project/$config_name.$config_suffix
 *    - $usr_subdir/$project/$config_name.$config_suffix.d/ *.$config_suffix
 *    - /run/$project/$config_name.$config_suffix.d/ *.$config_suffix
 *    - /etc/$project/$config_name.$config_suffix.d/ *.$config_suffix
 *
 *  No main $config_name.$config_suffix file is defined or must not be parsed:
 *
 *    - $usr_subdir/$project.d/ *.$config_suffix
 *    - /run/$project.d/ *.$config_suffix
 *    - /etc/$project.d/ *.$config_suffix
 *
 * For each parsed file the user defined function will be called in order
 * e.g. to check the correct file permissions.
 *
 * This call fulfills all requirements, defined by the Linux Userspace API (UAPI) Group
 * chapter "Configuration Files Specification".
 * See: https://uapi-group.org/specifications/specs/configuration_files_specification/
 *
 * @param key_file content of parsed file(s). MUST be initialized (e.g. with NULL).
 * @param project name of the project used as subdirectory, can be NULL
 * @param usr_subdir absolute path of the first directory (often "/usr/lib")
 * @param config_name basename of the configuration file
 *        If it is NULL, drop-ins without a main configuration file will
 *        be parsed only.
 * @param config_suffix suffix of the configuration file. Can also be NULL.
 * @param delim delimiters of key/value e.g. "\t ="
 *        If delim contains space characters AND none space characters,
 *        multiline values are not parseable.
 * @param comment array of characters which define the start of a comment
 * @param callback function which will be called for each file. This user defined function has the
 *        pathname as parameter and returns true if this file can be parsed. If not, the parsing of
 *        all files will be aborted and ECONF_PARSING_CALLBACK_FAILED will be returned.
 * @param callback_data pointer which will be given to the callback function.
 * @return econf_err ECONF_SUCCESS or error code
 *
 * Example: Reading content in different cases in following order:
 *
 *  /etc/foo/example.conf does exist:
 *
 *  - /etc/foo/example.conf
 *  - /usr/lib/foo/example.conf.d/ *.conf
 *  - /run/foo/example.conf.d/ *.conf
 *  - /etc/foo/example.conf.d/ *.conf
 *
 *  /etc/foo/example.conf does NOT exist:
 *
 *    /run/foo/example.conf does exist:
 *
 *    - /run/foo/example.conf
 *    - /usr/lib/foo/example.conf.d/ *.conf
 *    - /run/foo/example.conf.d/ *.conf
 *    - /etc/foo/example.conf.d/ *.conf
 *
 *    /run/foo/example.conf does NOT exist:
 *
 *    - /usr/lib/foo/example.conf
 *    - /usr/lib/foo/example.conf.d/ *.conf
 *    - /run/foo/example.conf.d/ *.conf
 *    - /etc/foo/example.conf.d/ *.conf
 *
 * @code
 *   #include "libeconf.h"
 *
 *   bool checkFile(const char *filename, const void *data) {
 *      - checking code which returns true or false -
 *	return true;
 *   }
 *
 *   econf_file *key_file = NULL;
 *   econf_err error;
 *
 *   error = econf_readConfigWithCallback (&key_file,
 *                                         "foo",
 *                                         "/usr/lib",
 *                                         "example",
 *                                         "conf",
 *                                         "=", "#",
 *                                         checkFile,
 *                                         NULL);
 *
 *   econf_free (key_file);
 *   key_file = NULL;
 * @endcode
 *
 * It is also possible to set additional options for e.g. parsing
 * config files which has python style format or defining the order
 * how to parse the configuration files. For more information have a look to
 * econf_newKeyFile_with_options.
 *
 */
extern econf_err econf_readConfigWithCallback(econf_file **key_file,
					      const char *project,
					      const char *usr_subdir,
					      const char *config_name,
					      const char *config_suffix,
					      const char *delim,
					      const char *comment,
					      bool (*callback)(const char *filename, const void *data),
					      const void *callback_data);


/** @brief Evaluating key/values of a given configuration by reading and merging all
 *         needed/available files in two different directories (normally in /usr/etc and /etc).
 *         DEPRECATED: Use the econf_readConfig/econf_readConfigWithCallback instead.
 *
 * @param key_file content of parsed file(s).
 * @param usr_conf_dir absolute path of the first directory (normally "/usr/etc")
 * @param etc_conf_dir absolute path of the second directory (normally "/etc")
 * @param config_name basename of the configuration file
 * @param config_suffix suffix of the configuration file. Can also be NULL.
 * @param delim delimiters of key/value e.g. "\t ="
 *        If delim contains space characters AND none space characters,
 *        multiline values are not parseable.
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
 *   key_file = NULL;
 * @endcode
 *
 */
extern econf_err __attribute__ ((deprecated("Use the econf_readConfig/econf_readConfigWithCallback instead")))
econf_readDirs(econf_file **key_file,
	       const char *usr_conf_dir,
	       const char *etc_conf_dir,
	       const char *config_name,
	       const char *config_suffix,
	       const char *delim,
	       const char *comment);

/** @brief Evaluating key/values for every given configuration files in two different
 *  directories (normally in /usr/etc and /etc). For each parsed file the user defined function
 *  will be called in order e.g. to check the correct file permissions.
 *  DEPRECATED: Use the econf_readConfig/econf_readConfigWithCallback instead.
 *
 * @param key_file content of parsed file(s).
 * @param usr_conf_dir absolute path of the first directory (normally "/usr/etc")
 * @param etc_conf_dir absolute path of the second directory (normally "/etc")
 * @param config_name basename of the configuration file
 * @param config_suffix suffix of the configuration file. Can also be NULL.
 * @param delim delimiters of key/value e.g. "\t ="
 *        If delim contains space characters AND none space characters,
 *        multiline values are not parseable.
 * @param comment array of characters which define the start of a comment
 * @param callback function which will be called for each file. This user defined function has the
 *        pathname as parameter and returns true if this file can be parsed. If not, the parsing of
 *        all files will be aborted and ECONF_PARSING_CALLBACK_FAILED will be returned.
 * @param callback_data pointer which will be given to the callback function.
 * @return econf_err ECONF_SUCCESS or error code
 *
 * Example: Reading content of example.conf in /usr/etc and /etc directory.
 * @code
 *   #include "libeconf.h"
 *
 *   bool checkFile(const char *filename, const void *data) {
 *      - checking code which returns true or false -
 *	return true;
 *   }
 *
 *   econf_file *key_file = NULL;
 *   econf_err error;
 *
 *   error = econf_readDirsWithCallback (&key_file,
 *                                       "/usr/etc",
 *                                       "/etc",
 *                                       "example",
 *                                       "conf",
 *                                       "=", "#",
 *                                       checkFile,
 *                                       NULL);
 *
 *   econf_free (key_file);
 *   key_file = NULL;
 * @endcode
 *
 */
extern econf_err __attribute__ ((deprecated("Use the econf_readConfig/econf_readConfigWithCallback instead")))
econf_readDirsWithCallback(econf_file **key_file,
			   const char *usr_conf_dir,
			   const char *etc_conf_dir,
			   const char *config_name,
			   const char *config_suffix,
			   const char *delim,
			   const char *comment,
			   bool (*callback)(const char *filename, const void *data),
			   const void *callback_data);

/** @brief Evaluating key/values for every given configuration files in two different
 *  directories (normally in /usr/etc and /etc). Returns a list of read configuration
 *  files and their values.
 *
 * @param key_files list of parsed file(s).
 *        Each entry includes all key/value, path, comments,... information of the regarding file.
 * @param size Size of the evaluated key_files list.
 * @param usr_conf_dir absolute path of the first directory (normally "/usr/etc")
 * @param etc_conf_dir absolute path of the second directory (normally "/etc")
 * @param config_name basename of the configuration file
 * @param config_suffix suffix of the configuration file. Can also be NULL.
 * @param delim delimiters of key/value e.g. "\t ="
 *        If delim contains space characters AND none space characters,
 *        multiline values are not parseable.
 * @param comment array of characters which define the start of a comment
 * @return econf_err ECONF_SUCCESS or error code
 *
 */
extern econf_err econf_readDirsHistory(econf_file ***key_files,
				       size_t *size,
				       const char *usr_conf_dir,
				       const char *etc_conf_dir,
				       const char *config_name,
				       const char *config_suffix,
				       const char *delim,
				       const char *comment);

/** @brief Evaluating key/values for every given configuration files in two different
 *  directories (normally in /usr/etc and /etc). For each parsed file the user defined function
 *  will be called in order e.g. to check the correct file permissions.
 *  Returns a list of read configuration files and their values.
 *
 * @param key_files list of parsed file(s).
 *        Each entry includes all key/value, path, comments,... information of the regarding file.
 * @param size Size of the evaluated key_files list.
 * @param usr_conf_dir absolute path of the first directory (normally "/usr/etc")
 * @param etc_conf_dir absolute path of the second directory (normally "/etc")
 * @param config_name basename of the configuration file
 * @param config_suffix suffix of the configuration file. Can also be NULL.
 * @param delim delimiters of key/value e.g. "\t ="
 *        If delim contains space characters AND none space characters,
 *        multiline values are not parseable.
 * @param comment array of characters which define the start of a comment
 * @param callback function which will be called for each file. This user defined function has the
 *        pathname as parameter and returns true if this file can be parsed. If not, the parsing of
 *        all files will be aborted and ECONF_PARSING_CALLBACK_FAILED will be returned.
 * @param callback_data pointer which will be given to the callback function.
 * @return econf_err ECONF_SUCCESS or error code
 *
 */
extern econf_err econf_readDirsHistoryWithCallback(econf_file ***key_files,
						   size_t *size,
						   const char *usr_conf_dir,
						   const char *etc_conf_dir,
						   const char *config_name,
						   const char *config_suffix,
						   const char *delim,
						   const char *comment,
						   bool (*callback)(const char *filename, const void *data),
						   const void *callback_data);

/** @brief Create a new econf_file object.
 *
 * @param result Pointer to the allocated econf_file object.
 * @param delimiter delimiter of key/value e.g. "="
 * @param comment Character which defines the start of a comment.
 * @return econf_err ECONF_SUCCESS or error code
 *
 */
extern econf_err econf_newKeyFile(econf_file **result, char delimiter, char comment);

/** @brief Create a new econf_file object with special options.
 *
 * @param result Pointer to the allocated econf_file object.
 * @param options defined as a string separated by ";"
 *        format "<key>=<value>"; e.g. "JOIN_SAME_ENTRIES=1"
 * @return econf_err ECONF_SUCCESS or error code
 *
 * Following options are supported:
 *  JOIN_SAME_ENTRIES  (default 0)
 *    Parsed entries with the same name will not be replaces but
 *    will be joined to one entry.
 *  PYTHON_STYLE  (default 0)
 *    E.G. Identations will be handled like multiline entries.
 *  PARSING_DIRS (default /usr/etc/:/run:/etc)
 *    List of directories from which the configuration files have to be parsed.
 *    The list is a string, divides by ":". The last entry has the highest
 *    priority. E.g.: "PARSING_DIRS=/usr/etc/:/run:/etc"
 *  CONFIG_DIRS (default \<empty\>)
 *    List of directory structures (with order) which describes the directories
 *    in which the files have to be parsed.
 *    The list is a string, divides by ":". The last entry has the highest
 *    priority. E.g. with the given list: "CONFIG_DIRS=.conf.d:.d" files in following
 *    directories will be parsed:
 *           "<default_dirs>/<config_name>.conf.d/"
 *           "<default_dirs>/<config_name>.d/"
 *           "<default_dirs>/<config_name>/"
 *
 * e.g. Parsing configuration files written in python style:
 *
 * @code
 *   #include "libeconf.h"
 *
 *   econf_file *key_file = NULL;
 *   econf_err error;
 *
 *   if (error = econf_newKeyFile_with_options(&key_file, "PYTHON_STYLE=1"))
 *   {
 *     fprintf (stderr, "ERROR: couldn't create new key file: %s\n",
 *              econf_errString(error));
 *     return 1;
 *   }
 *
 *   error = econf_readConfig (&key_file,
 *                             "foo",
 *                             "/usr/lib",
 *                             "example",
 *                             "conf",
 *                             "=", "#");
 *
 *   econf_free (key_file);
 *   key_file = NULL;
 * @endcode
 */
extern econf_err econf_newKeyFile_with_options(econf_file **result, const char *options);

/** @brief Create a new econf_file object in IniFile format. So the delimiter
 *         will be "=" and comments are beginning with "#".
 *
 * @param result Pointer to the allocated econf_file object.
 * @return econf_err ECONF_SUCCESS or error code
 *
 */
extern econf_err econf_newIniFile(econf_file **result);

/** @brief Returns the comment character tag of the given econf_file object.
 *         This tag will be taken while writing comments to file.
 *
 * @param key_file econf_file object.
 * @return char comment character tag
 *
 */
extern char econf_comment_tag(econf_file *key_file);

/** @brief Returns the delimiter character of the given econf_file object.
 *         This delimiter will be taken while writing the data to file.
 *
 * @param key_file econf_file object.
 * @return char delimiter of key/value
 *
 */
extern char econf_delimiter_tag(econf_file *key_file);

/** @brief Set the comment character tag of the given econf_file object.
 *         This tag will be taken while writing comments to file.
 *
 * @param key_file econf_file object.
 * @param comment comment tag
 *
 */
extern void econf_set_comment_tag(econf_file *key_file, const char comment);

/** @brief Set the delimiter character of the given econf_file object.
 *         This delimiter will be taken while writing the data to file.
 *
 * @param key_file econf_file object.
 * @param delimiter delimiter of key/value
 *
 */
extern void econf_set_delimiter_tag(econf_file *key_file, const char delimiter);

/** @brief Write content of an econf_file struct to specified location.
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

/** @brief Evaluating path name.
 *
 * @param kf given/parsed data
 * @return Absolute path name or an empty string if kf is a result of
 *         already merged data (e.G. returned by econf_readDirs).
 *
 */
extern char *econf_getPath(econf_file *kf);

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

/** @brief All parsed files require this user permission.
 *         DEPRECATED: Use the callback in econf_readFileWithCallback or
 *         econf_readConfigWithCallback instead.
 *
 * @param owner User ID
 * @return void
 *
 */
extern void __attribute__ ((deprecated("use callback in econf_readFileWithCallback or econf_readConfigWithCallback instead")))
econf_requireOwner(uid_t owner);

/** @brief All parsed files require this group permission.
 *         DEPRECATED: Use the callback in econf_readFileWithCallback,
 *         econf_readDirsWithCallback or econf_readDirsHistoryWithCallback
 *         instead.
 *
 * @param group Group ID
 * @return void
 *
 */
extern void __attribute__ ((deprecated("use callback in econf_readFileWithCallback or econf_readConfigWithCallback instead")))
econf_requireGroup(gid_t group);

/** @brief All parsed file have to have these file and directory permissions.
 *         DEPRECATED: Use the callback in econf_readFileWithCallback or
 *         econf_readConfigWithCallback instead.
 *
 * @param file_perms file permissions
 * @param dir_perms dir permissions
 * @return void
 *
 */
extern void __attribute__ ((deprecated("use callback in econf_readFileWithCallback or econf_readConfigWithCallback instead")))
econf_requirePermissions(mode_t file_perms, mode_t dir_perms);

/** @brief Allowing the parser to follow sym links (default: true).
 *         DEPRECATED: Use the callback in econf_readFileWithCallback or
 *         econf_readConfigWithCallback instead.
 *
 * @param allow allow to follow sym links.
 * @return void
 *
 */
extern void __attribute__ ((deprecated("use callback in econf_readFileWithCallback or econf_readConfigWithCallback instead")))
econf_followSymlinks(bool allow);

/** @brief Reset all UID, GID, permissions,... restrictions for parsed files/dirs.
 *         DEPRECATED: Use the callback in econf_readFileWithCallback or
 *         econf_readConfigWithCallback instead.
 *         instead.
 *
 * @return void
 *
 */
extern void __attribute__ ((deprecated("use callback in econf_readFileWithCallback or econf_readConfigWithCallback instead")))
econf_reset_security_settings(void);

/** @brief Set a list of directory structures (with order) which describes the directories
 *         in which the files have to be parsed.
 *
 * @param dir_postfix_list list of directory structures.
 *        E.G. with the given list: {"conf.d", ".d", "/", NULL} files in following
 *        directories will be parsed:
 *           "<default_dirs>/<config_name>.conf.d/"
 *           "<default_dirs>/<config_name>.d/"
 *           "<default_dirs>/<config_name>/"
 *
 * @return econf_err ECONF_SUCCESS or error code
 *
 * CAUTION: These options are NOT TRHEAD-SAFE because they are set
 *          globally in libeconf. Individual setting set with econf_newKeyFile_with_options
 *          have higher priority and are trhead-safe.
 *
 */
extern econf_err __attribute__ ((deprecated("Is not thread-safe. Use econf_newKeyFile_with_options instead")))
econf_set_conf_dirs(const char **dir_postfix_list);

#ifdef __cplusplus
}
#endif
