/*
  Copyright (C) 2021 SUSE LLC

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
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include "libeconf.h"
#include "helpers.h"
#include "keyfile.h"
#include "libeconf_ext.h"

static char *ltrim(char *s)
{
  while(isspace(*s)) s++;
  return s;
}

static char *rtrim(char *s)
{
  if (strlen(s)<=0)
    return s;

  char* back = s + strlen(s);
  while(isspace(*--back));
  *(back+1) = '\0';
  return s;
}

static char *trim(char *s)
{
  return rtrim(ltrim(s));
}

econf_err
econf_getExtValue(econf_file *kf, const char *group,
		  const char *key, econf_ext_value **result)
{
  if (!kf)
    return ECONF_ERROR;

  size_t num;
  econf_err error = find_key(*kf, group, key, &num);
  if (error)
    return error;

  *result = malloc(sizeof(econf_ext_value));
  if (*result==NULL)
    return ECONF_NOMEM;

  getCommentsNum(*kf, num,
		 &((*result)->comment_before_key),
		 &((*result)->comment_after_value));
  getPath(*kf, &((*result)->file));
  getLineNrNum(*kf, num, &((*result)->line_number));

  char *value_string = NULL;
  getStringValueNum(*kf, num, &value_string);

  char buf[BUFSIZ];
  char *line;
  size_t n_del = 0;

  (*result)->values = NULL;

  if (value_string!=NULL) {
    strncpy(buf,value_string,BUFSIZ-1);
    buf[BUFSIZ-1] = '\0';
    free(value_string);
    value_string = trim(buf);

    if (value_string[0] == '\"')
    {
      /* one quoted string only */
      (*result)->values = realloc ((*result)->values, sizeof (char*) * ++n_del);
      if ((*result)->values == NULL) {
        econf_freeExtValue(*result);
        return ECONF_NOMEM; /* memory allocation failed */
      }
      (*result)->values[n_del-1] = strdup(value_string);
    } else {
      /* splitting into a character array */
      while ((line = strsep(&value_string, "\n")) != NULL) {
        (*result)->values = realloc ((*result)->values, sizeof (char*) * ++n_del);
        if ((*result)->values == NULL) {
          econf_freeExtValue(*result);
          return ECONF_NOMEM; /* memory allocation failed */
        }
        (*result)->values[n_del-1] = strdup(trim(line));
      }
    }
  }

  /* realloc one extra element for the last 0 */
  (*result)->values = realloc ((*result)->values, sizeof (char*) * ++n_del);
  (*result)->values[n_del-1] = 0;

  return ECONF_SUCCESS;
}

extern void econf_freeExtValue(econf_ext_value *to_free)
{
  if (!to_free) { return; }

  /* freeing array of strings */
  char **str = to_free->values;
  while (*str)
    free(*str++); 
  free(to_free->values);

  free(to_free->file);
  free(to_free->comment_before_key);
  free(to_free->comment_after_value);
  free(to_free);
}
