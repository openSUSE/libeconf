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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include "libeconf.h"
#include "getfilecontents.h"

static const char *messages[] = {
  "Success", /* ECONF_SUCCESS */
  "Unknown error", /* ECONF_ERROR */
  "Out of memory", /* ECONF_NOMEM */
  "Configuration file not found", /*ECONF_NOFILE */
  "Group not found", /* ECONF_NOGROUP */
  "Key not found", /* ECONF_NOKEY */
  "Key is NULL or has empty value", /* ECONF_EMPTYKEY */
  "Error creating or writing to a file", /* ECONF_WRITEERROR */
  "Parse error", /* ECONF_PARSE_ERROR */
  "Missing bracket", /* ECONF_MISSING_BRACKET */
  "Missing delimiter", /* ECONF_MISSING_DELIMITER */
  "Empty section name", /* ECONF_EMPTY_SECTION_NAME */
  "Text after section", /* ECONF_TEXT_AFTER_SECTION */
  "Conf file list is NULL", /* ECONF_FILE_LIST_IS_NULL */
  "Wrong boolean value (1/0 true/false yes/no)", /* ECONF_WRONG_BOOLEAN_VALUE */
  "Given key has NULL value", /* ECONF_KEY_HAS_NULL_VALUE */
  "File has wrong owner", /* ECONF_WRONG_OWNER */
  "File has wrong group", /* ECONF_WRONG_GROUP */
  "File has wrong file permissions", /* ECONF_WRONG_FILE_PERMISSION */
  "File has wrong dir permissions", /* ECONF_WRONG_DIR_PERMISSION */
  "File is a sym link which is not permitted", /* ECONF_ERROR_FILE_IS_SYM_LINK */
  "User defined parsing callback has failed", /* ECONF_PARSING_CALLBACK_FAILED */
  "Given argument is NULL", /* ECONF_ARGUMENT_IS_NULL_VALUE */
  "Given option not found", /* ECONF_OPTION_NOT_FOUND */
  "Value cannot be converted", /* ECONF_VALUE_CONVERSION_ERROR */
};

const char *
econf_errString (const econf_err error)
{
  if (error >= sizeof(messages)/sizeof(messages[0]))
    {
      static char buffer[1024]; /* should always be big enough, else truncate */
      const char *unknown = "Unknown libeconf error %i";

      snprintf (buffer, sizeof (buffer), unknown, error);

      return buffer;
    }
  else
    return messages[error];
}

extern void econf_errLocation (char **filename, uint64_t *line_nr)
{
  last_scanned_file( filename, line_nr );
}
