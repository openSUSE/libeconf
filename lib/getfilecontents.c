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
#include "getfilecontents.h"
#include "helpers.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*info for reporting scan errors (line Nr, filename) */
static uint64_t last_scanned_line_nr = 0;
static char last_scanned_filename[PATH_MAX];

static econf_err
join_same_entries(econf_file *ef)
{
  for(size_t i = 0; i < ef->length; i++)
  {
    for(size_t j = i+1; j < ef->length; j++)
    {
      if (strcmp(ef->file_entry[i].group, ef->file_entry[j].group) == 0 &&
	  strcmp(ef->file_entry[i].key, ef->file_entry[j].key) == 0)
      {
	char *post, *pre;
	if (ef->file_entry[j].value == NULL ||
	    strlen(ef->file_entry[j].value) == 0)
	{
	  /* reset entry */
	  free(ef->file_entry[i].value);
	  ef->file_entry[i].value = strdup("");
	} else {
	  /* appending value */
	  post = ef->file_entry[j].value;
	  pre = ef->file_entry[i].value;
	  int ret = 0;
	  if (post != NULL && strlen(post) > 0)
	  {
	    /* removing leading spaces */
	    while(isspace(*post)) post++;
	    ret = asprintf(&(ef->file_entry[i].value), "%s\n%s", pre,
			   post);
	    if(ret<0)
	      return ECONF_NOMEM;
	    free(pre);
	  }
	}

	/* appending before key comment */
	if (ef->file_entry[j].comment_before_key != NULL &&
	    strlen(ef->file_entry[j].comment_before_key) > 0)
	{
	  post = ef->file_entry[j].comment_before_key;
          pre = ef->file_entry[i].comment_before_key;
	  int ret = asprintf(&(ef->file_entry[i].comment_before_key),
			     "%s\n%s", pre, post);
	  if(ret<0)
	    return ECONF_NOMEM;
	  free(pre);
	}

	if (ef->file_entry[j].value == NULL ||
	    strlen(ef->file_entry[j].value) == 0)
	{
	  /* reset after value comment */
	  free(ef->file_entry[i].comment_after_value);
	  ef->file_entry[i].comment_after_value = NULL;
	} else {
	  /* appending after value comment */
	  if (ef->file_entry[j].comment_after_value != NULL &&
	      strlen(ef->file_entry[j].comment_after_value) > 0)
	  {
	    post = ef->file_entry[j].comment_after_value;
	    pre = ef->file_entry[i].comment_after_value;
	    /* removing leading spaces */
            while(isspace(*post)) post++;
            if (pre == NULL)
	    {
	      ef->file_entry[i].comment_after_value = strdup(post);
	    } else {
	      int ret = asprintf(&(ef->file_entry[i].comment_after_value),
				 "%s\n%s", pre, post);
	      if(ret<0)
		return ECONF_NOMEM;
	      free(pre);
	    }
	  }
	}
      }
    }
  }
  return ECONF_SUCCESS;
}

static econf_err
store (econf_file *ef, const char *group, const char *key,
       const char *value, const uint64_t line_number,
       const char *comment_before_key, const char *comment_after_value,
       const bool quotes,
       const bool append_entry)
{
  if (append_entry)
  {
    /* Appending next line to the last entry. */
    if (ef->length<=0)
    {
      return ECONF_MISSING_DELIMITER;
    }
    char *content = ef->file_entry[ef->length-1].value;
    int ret = asprintf(&(ef->file_entry[ef->length-1].value), "%s\n%s", content,
	     value);
    if(ret<0)
      return ECONF_NOMEM;
    free(content);
    /* Points to the end of the array. This is needed for the next entry. */
    ef->file_entry[ef->length-1].line_number = line_number;

    if (ef->file_entry[ef->length-1].comment_after_value &&
	!comment_after_value)
    { /* multiline entry. This line has no comment. So we have to add an empty entry. */
      comment_after_value = "";
    }

    if (comment_after_value)
    {
      ret = -1;
      if (ef->file_entry[ef->length-1].comment_after_value)
      {
	content = ef->file_entry[ef->length-1].comment_after_value;
	ret = asprintf(&(ef->file_entry[ef->length-1].comment_after_value), "%s\n%s", content,
		       comment_after_value);
	free(content);
      } else {
	ret = asprintf(&(ef->file_entry[ef->length-1].comment_after_value), "\n%s",
		       comment_after_value);
      }
      if(ret<0)
	return ECONF_NOMEM;
    }      

    return ECONF_SUCCESS;
  }

  /* not appending -> new entry */
  if (ef->alloc_length == ef->length) {
    struct file_entry *tmp;

    tmp = realloc(ef->file_entry, (ef->length + 1) * sizeof(struct file_entry));
    if (!tmp)
      return ECONF_NOMEM;
    ef->file_entry = tmp;
    ef->length++;
    ef->alloc_length = ef->length;
  }

  ef->file_entry[ef->length-1].line_number = line_number;

  ef->file_entry[ef->length-1].quotes = quotes;

  if (group)
    ef->file_entry[ef->length-1].group = strdup(group);
  else
    ef->file_entry[ef->length-1].group = strdup(KEY_FILE_NULL_VALUE);

  if (key) {
    /* remove space at the end of the key */
    const char *p = key + strlen(key);
    if (p > key)
      p--;
    while (p > key && (isspace((unsigned)*p)))
      p--;
    ef->file_entry[ef->length-1].key = strndup(key, (size_t)(p+1-key));
  }
  else
    ef->file_entry[ef->length-1].key = strdup(KEY_FILE_NULL_VALUE);

  if (value)
    ef->file_entry[ef->length-1].value = strdup(value);
  else
    ef->file_entry[ef->length-1].value = NULL;

  if (comment_before_key)
    ef->file_entry[ef->length-1].comment_before_key = strdup(comment_before_key);
  else
    ef->file_entry[ef->length-1].comment_before_key = NULL;
  if (comment_after_value)
    ef->file_entry[ef->length-1].comment_after_value = strdup(comment_after_value);
  else
    ef->file_entry[ef->length-1].comment_after_value = NULL;

  return ECONF_SUCCESS;
}

static void
check_delim(const char *str, bool *has_wsp, bool *has_nonwsp)
{
  const char *p;
  *has_wsp = *has_nonwsp = false;

  if (str == NULL)
    return;
  for (p = str; *p && !(*has_wsp && *has_nonwsp); p++) {
    if (isspace((unsigned)*p))
      *has_wsp = true;
    else
      *has_nonwsp = true;
  }
}

static void free_buffer(char **buffer)
{
    free(*buffer);
}

/* Read the file line by line and parse for comments, keys and values */
econf_err
read_file(econf_file *ef, const char *file,
	  const char *delim, const char *comment)
{
  char *current_group = NULL;
  char *current_comment_before_key = NULL;
  char *current_comment_after_value = NULL;
  econf_err retval = ECONF_SUCCESS;
  uint64_t line = 0;
  bool has_wsp, has_nonwsp;
  FILE *kf = fopen(file, "rbe");

  if (kf == NULL)
    return ECONF_NOFILE;

  snprintf(last_scanned_filename, sizeof(last_scanned_filename), "%s", file);

  check_delim(delim, &has_wsp, &has_nonwsp);

  ef->path = strdup (file);
  if (ef->path == NULL) {
    fclose (kf);
    return ECONF_NOMEM;
  }
  ef->delimiter = *delim;

  char *buf = malloc(BUFSIZ*sizeof(char));
  size_t max_size = BUFSIZ;
  while (getline(&buf, &max_size, kf) >0) {
    char *p, *name, *data = NULL;
    bool quote_seen = false, delim_seen = false;
    char *org_buf __attribute__ ((__cleanup__(free_buffer))) = strdup(buf);

    line++;
    last_scanned_line_nr = line;

    /* Remove trailing newline character */
    size_t n = strlen(buf);
    if (n && *(buf + n - 1) == '\n')
      *(buf + n - 1) = '\0';

    if (!*buf)
      continue; /* empty line */

    /* ignore space at begin of the line */
    name = buf;
    while (*name && isspace((unsigned)*name))
      name++;

    /* go through all comment characters and check if one of them could be found */
    for (size_t i = 0; i < strlen(comment); i++) {
      p = strchr(name, comment[i]);
      if (p)
      {
	if(p==name)
	{
	  /* Comment is defined in the line before the key/value line */
	  if (current_comment_before_key)
          {
	    /* appending */
	    char *content = current_comment_before_key;
	    int ret = asprintf(&current_comment_before_key, "%s\n%s", content,
			       p+1);
	    if(ret<0) {
	      free(buf);
	      return ECONF_NOMEM;
	    }
	    free(content);
	  } else {
	    current_comment_before_key = strdup(p+1);
	  }
	} else {
	  /* Comment is defined after the key/value in the same line */
	  if (current_comment_after_value)
	  {
	    /* appending */
	    char *content = current_comment_after_value;
	    int ret = asprintf(&current_comment_after_value, "%s\n%s", content,
			       p+1);
	    if(ret<0) {
	      free(buf);
	      return ECONF_NOMEM;
	    }
	    free(content);
	  } else {
	    current_comment_after_value = strdup(p+1);
	  }
	}
	*p = '\0';
      }
    }

    if (!*buf)
      continue; /* result is empty line */

    /* check for groups */
    if (name[0] == '[') {
      name++; /* remove "[" */
      p = name + strlen(name) -1;
      while (isspace (*p)) p--;
      if (*p != ']') {
	if (strchr(name,']') == NULL)
	  retval = ECONF_MISSING_BRACKET;
	else
	  retval = ECONF_TEXT_AFTER_SECTION;
	goto out;
      }
      *p = '\0'; /* remove "]" */
      if(strlen(name) <= 0)
      {
	retval = ECONF_EMPTY_SECTION_NAME;
	goto out;
      }
      if (current_group)
	free (current_group);
      current_group = strdup (name);
      continue;
    }

    if (delim == NULL || strlen(delim) == 0 || strcmp(delim, "\n") == 0) {
      /* No delimiter is defined. Key without a value will be stored. */
      retval = store(ef, current_group, name, data, line,
		     current_comment_before_key, current_comment_after_value,
		     false, /* no quote */
		     false /* new entry */);
      free(current_comment_before_key);
      current_comment_before_key = NULL;
      free(current_comment_after_value);
      current_comment_after_value = NULL;
      continue;
    }

    /* Valid delimiters are defined */
    /* go to the end of the name */
    data = name;
    while (*data && !(isspace((unsigned)*data) ||
		      strchr(delim, *data) != NULL))
      data++;
    if (data > name && *data) {
      if (has_wsp && has_nonwsp)
      {
	/*
	 * delim contains both whitespace and non-whitespace characters.
	 * In this case delim_seen has the special meaning "non-whitespace
	 * delim seen". See comment below.
	 */
	delim_seen = !isspace((unsigned)*data) &&
	  strchr(delim, *data) != NULL;
      }
      else
      {
	delim_seen = strchr(delim, *data) != NULL;
      }
      *data++ = '\0';
    }

    if (!has_wsp || !has_nonwsp) {
      /* Multiline entries make no sense if the delimiters are
       * whitespaces AND none whitespaces.
       */

      /* Checking and adding multiline entries which are
       * not defined by a beginning quote in the line before.
       */
      bool found_delim = delim_seen;
      if (!found_delim)
      {
        /* searching the rest of the string for delimiters */
	char *c = data;
	while (*c && !(strchr(delim, *c) != NULL))
	  c++;
        if (*c)
	  found_delim = true;
      }
      if (!found_delim &&
	  /* Entry has already been found */
	  ef->length > 0 &&
	  /* The Entry must be the next line. Otherwise it is a new one */
	  ef->file_entry[ef->length-1].line_number+1 == line)
      {
        /* removing comments */
        for (size_t i = 0; i < strlen(comment); i++) {
	  char *pt = strchr(org_buf, comment[i]);
	  if (pt)
	    *pt = '\0';
	}
	/* removing \n at the end of the line */
	if( org_buf[strlen(org_buf)-1] == '\n' )
	  org_buf[strlen(org_buf)-1] = 0;
	retval = store(ef, current_group, name, org_buf, line,
		       current_comment_before_key, current_comment_after_value,
		       false, /* Quotes does not matter in the following lines */
		       true /* appending entry */);
	free(current_comment_before_key);
	current_comment_before_key = NULL;
	free(current_comment_after_value);
	current_comment_after_value = NULL;
	if (retval)
	  goto out;
	continue;
      }
    }
    
    /* Go on. It is not a multiline entry */
    
    if (!*name || data == name)
      continue;

    if (*data == '\0')
      /* No separator -> return NULL pointer, there is no value,
	 not even an empty key */
      data = NULL;
    else {
      /* go to the beginning of the value */
      while (*data && isspace((unsigned)*data))
	data++;
      if (!has_wsp && !delim_seen) {
	/*
	 * If delim consists only of non-whitespace characters,
	 * require at least one delimiter, and skip more whitespace
	 * after it.
	 */
	if (!*data || strchr(delim, *data) == NULL) {
	  retval = ECONF_MISSING_DELIMITER;
	  goto out;
	}
	data++;
	while (*data && isspace((unsigned)*data))
	  data++;
      } else if (has_wsp && has_nonwsp && !delim_seen &&
		 *data && strchr(delim, *data) != NULL) {
	/*
	 * If delim contains both whitespace and non-whitespace characters,
	 * use any combination of one non-whitespace delimiter and
	 * many whitespace delimiters as separator, but not multiple
	 * non-whitespace delimiters. Example with delim = " =":
	 * key value -> "value"
	 * key=value -> "value"
	 * key = value -> "value"
	 * key=  value -> "value"
	 * key == value -> "= value"
	 * key==value -> "=value"
	 */
	data++;
	while (*data && isspace((unsigned)*data))
	  data++;
      }
      if (*data == '"') {
	quote_seen = true;
	data++;
      }

      /* remove space at the end of the value */
      p = data + strlen(data);
      if (p > data)
	p--;
      while (p > data && (isspace((unsigned)*p)))
	p--;
      /* Strip double quotes only if both leading and trailing quotes exist. */
      if (p >= data && quote_seen) {
	if (*p == '"')
	  p--;
	else
	  data--;
      }
      if (*p != '\0' && *(p + 1) != '\0')
	*(p + 1) = '\0';
    }

    retval = store(ef, current_group, name, data, line,
		   current_comment_before_key, current_comment_after_value,
		   quote_seen,
		   false /* new entry */);
    free(current_comment_before_key);
    current_comment_before_key = NULL;
    free(current_comment_after_value);
    current_comment_after_value = NULL;    
    if (retval)
      goto out;
  }

 out:
  free(buf);
  fclose (kf);
  if (current_group)
    free (current_group);
  if (current_comment_before_key)
    free(current_comment_before_key);
  if (current_comment_after_value)
    free(current_comment_after_value);

  if(getenv("ECONF_JOIN_SAME_ENTRIES"))
  {
    join_same_entries(ef);
  }

  return retval;
}

void last_scanned_file(char **filename, uint64_t *line_nr)
{
  *line_nr = last_scanned_line_nr;
  *filename = strdup(last_scanned_filename);
}
