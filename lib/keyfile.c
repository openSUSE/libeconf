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

#include "libeconf.h"
#include "defines.h"
#include "helpers.h"
#include "keyfile.h"

#include <errno.h>
#include <float.h>
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

void print_key_file(const econf_file key_file)
{
  printf("----------------------------------\n");
  printf("path: %s\n", key_file.path);
  printf("delimiter: %c, comment: %c\n", key_file.delimiter, key_file.comment);
  printf("values:\n");
  for(size_t i = 0; i < key_file.length; i++)
  {
    printf("  group: %s ; key: %s ; value: %s ; pre_comment %s ; added_comment: %s\n",
	   key_file.file_entry[i].group,
	   key_file.file_entry[i].key,
	   key_file.file_entry[i].value,
	   key_file.file_entry[i].comment_before_key,
	   key_file.file_entry[i].comment_after_value
	   );
  }
  printf("----------------------------------\n");
}


econf_err key_file_append(econf_file *kf) {
  if (kf == NULL)
    return ECONF_ERROR;
  if(kf->length++ >= kf->alloc_length) {
    kf->alloc_length++;
    kf->file_entry =
      realloc(kf->file_entry, (kf->alloc_length) * sizeof(struct file_entry));
    if (kf->file_entry == NULL)
      return ECONF_NOMEM;
    initialize(kf, kf->alloc_length - 1);
  }
  return ECONF_SUCCESS;
}

/* --- GETTERS --- */

econf_err getIntValueNum(econf_file key_file, size_t num, int32_t *result) {
  char *endptr;
  errno = 0;
  *result = strtol(key_file.file_entry[num].value, &endptr, 0);
  if (endptr == key_file.file_entry[num].value || errno == ERANGE || (errno != 0 && *result == 0))
    return ECONF_VALUE_CONVERSION_ERROR;
  return ECONF_SUCCESS;
}

econf_err getInt64ValueNum(econf_file key_file, size_t num, int64_t *result) {
  char *endptr;
  errno = 0;
  *result = strtoll(key_file.file_entry[num].value, &endptr, 0);
  if (endptr == key_file.file_entry[num].value || errno == ERANGE || (errno != 0 && *result == 0))
    return ECONF_VALUE_CONVERSION_ERROR;
  return ECONF_SUCCESS;
}

econf_err getUIntValueNum(econf_file key_file, size_t num, uint32_t *result) {
  char *endptr;
  errno = 0;
  *result = strtoul(key_file.file_entry[num].value, &endptr, 0);
  if (endptr == key_file.file_entry[num].value || errno == ERANGE || (errno != 0 && *result == 0))
    return ECONF_VALUE_CONVERSION_ERROR;
  return ECONF_SUCCESS;
}

econf_err getUInt64ValueNum(econf_file key_file, size_t num, uint64_t *result) {
  char *endptr;
  errno = 0;
  *result = strtoull(key_file.file_entry[num].value, &endptr, 0);
  if (endptr == key_file.file_entry[num].value || errno == ERANGE || (errno != 0 && *result == 0))
    return ECONF_VALUE_CONVERSION_ERROR;
  return ECONF_SUCCESS;
}

econf_err getFloatValueNum(econf_file key_file, size_t num, float *result) {
  char *endptr;
  errno = 0;
  *result = strtof(key_file.file_entry[num].value, &endptr);
  if (endptr == key_file.file_entry[num].value) /* do not check errno because it is a false alarm in ppc and S390 */
    return ECONF_VALUE_CONVERSION_ERROR;
  return ECONF_SUCCESS;
}

econf_err getDoubleValueNum(econf_file key_file, size_t num, double *result) {
  char *endptr;
  errno = 0;
  *result = strtod(key_file.file_entry[num].value, &endptr);
  if (endptr == key_file.file_entry[num].value || errno == ERANGE || (errno != 0 && *result == 0))
    return ECONF_VALUE_CONVERSION_ERROR;
  return ECONF_SUCCESS;
}

econf_err getStringValueNum(econf_file key_file, size_t num, char **result) {
  if (key_file.file_entry[num].value)
  {
    *result = strdup(key_file.file_entry[num].value);
    if (*result == NULL)
      return ECONF_NOMEM;
  } else {
    *result = NULL;
  }

  return ECONF_SUCCESS;
}

econf_err getBoolValueNum(econf_file key_file, size_t num, bool *result) {
  char *value, *tmp;
  tmp = strdup(key_file.file_entry[num].value);
  value = toLowerCase(tmp);
  size_t hash = hashstring(toLowerCase(key_file.file_entry[num].value));
  econf_err err = ECONF_SUCCESS;

  if ((*value == '1' && strlen(tmp) == 1) || hash == YES || hash == TRUE)
    *result = true;
  else if ((*value == '0' && strlen(tmp) == 1) || !*value ||
	   hash == NO || hash == FALSE)
    *result = false;
  else if (hash == KEY_FILE_NULL_VALUE_HASH)
    err = ECONF_KEY_HAS_NULL_VALUE;
  else
    err = ECONF_PARSE_ERROR;

  free(tmp);
  return err;
}

econf_err getCommentsNum(econf_file key_file, size_t num,
			 char **comment_before_key,
			 char **comment_after_value) {
  if (key_file.file_entry[num].comment_before_key)
    *comment_before_key = strdup(key_file.file_entry[num].comment_before_key);
  else
    *comment_before_key = NULL;

  if (key_file.file_entry[num].comment_after_value)
    *comment_after_value = strdup(key_file.file_entry[num].comment_after_value);
  else
    *comment_after_value = NULL;

  return ECONF_SUCCESS;
}

econf_err getLineNrNum(econf_file key_file, size_t num, uint64_t *line_nr) {
  *line_nr = key_file.file_entry[num].line_number;

  return ECONF_SUCCESS;
}

econf_err getPath(econf_file key_file, char **path) {
  if (key_file.path)
  {
    *path = strdup(key_file.path);
  } else {
    *path = NULL;
  }

  return ECONF_SUCCESS;
}

/* --- SETTERS --- */

econf_err setGroup(econf_file *key_file, size_t num, const char *value) {
  if (key_file == NULL || value == NULL)
    return ECONF_ERROR;

  key_file->file_entry[num].group = setGroupList(key_file, value);
  if (key_file->file_entry[num].group == NULL)
    return ECONF_NOMEM;

  return ECONF_SUCCESS;
}

econf_err setKey(econf_file *key_file, size_t num, const char *value) {
  if (key_file == NULL || value == NULL)
    return ECONF_ERROR;
  if (key_file->file_entry[num].key)
    free(key_file->file_entry[num].key);
  key_file->file_entry[num].key = strdup(value);
  if (key_file->file_entry[num].key == NULL)
    return ECONF_NOMEM;

  return ECONF_SUCCESS;
}

#define econf_setValueNum(FCT_TYPE, TYPE, FMT, PR)			\
econf_err set ## FCT_TYPE ## ValueNum(econf_file *ef, size_t num, const void *v) { \
  const TYPE *value = (const TYPE*) v; \
  char *ptr; \
\
  if (asprintf (&ptr, FMT PR, *value) == -1) \
    return ECONF_NOMEM; \
\
  if (ef->file_entry[num].value) \
    free(ef->file_entry[num].value); \
\
  ef->file_entry[num].value = ptr; \
\
  return ECONF_SUCCESS; \
}

econf_setValueNum(Int, int32_t, "%", PRId32)
econf_setValueNum(Int64, int64_t, "%",  PRId64)
econf_setValueNum(UInt, uint32_t, "%", PRIu32)
econf_setValueNum(UInt64, uint64_t, "%", PRIu64)
#define PRFLOAT ,FLT_DECIMAL_DIG
econf_setValueNum(Float, float, "%.*g", PRFLOAT)
#define PRDOUBLE ,DBL_DECIMAL_DIG
econf_setValueNum(Double, double, "%.*g", PRDOUBLE)

econf_err setStringValueNum(econf_file *ef, size_t num, const void *v) {
  const char *value = (const char*) (v ? v : "");
  char *ptr;

  if ((ptr = strdup (value)) == NULL)
    return ECONF_NOMEM;

  if (ef->file_entry[num].value)
    free(ef->file_entry[num].value);

  ef->file_entry[num].value = ptr;

  return ECONF_SUCCESS;
}

econf_err setBoolValueNum(econf_file *kf, size_t num, const void *v) {
  const char *value = (const char*) (v ? v : KEY_FILE_NULL_VALUE);
  econf_err error = ECONF_SUCCESS;
  char *tmp = strdup(value);
  size_t hash = hashstring(toLowerCase(tmp));

  if ((*value == '1' && strlen(tmp) == 1) || hash == YES || hash == TRUE) {
    free(kf->file_entry[num].value);
    kf->file_entry[num].value = strdup("true");
  } else if ((*value == '0' && strlen(tmp) == 1) ||
             hash == NO || hash == FALSE) {
    free(kf->file_entry[num].value);
    kf->file_entry[num].value = strdup("false");
  } else if (hash == KEY_FILE_NULL_VALUE_HASH || strlen(value) == 0) {
    free(kf->file_entry[num].value);
    kf->file_entry[num].value = strdup(KEY_FILE_NULL_VALUE);
  } else { error = ECONF_WRONG_BOOLEAN_VALUE; }

  free(tmp);
  return error;
}
