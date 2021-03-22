#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include "libeconf_ext.h"

/* Test case:
 *  Tests several string format. The result is a econf_ext_value
 *  struct. See tst-arguments-string/etc/arguments.conf.
 */

static int
print_error_get (const char *getgroup, const char *key, econf_err error)
{
  fprintf (stderr, "ERROR: tried to get '%s' from '%s': %s\n",
	   key, getgroup, econf_errString(error));
  return 1;
}

static bool
check_StringArray (econf_file *key_file, const char *group,
		   const char *key, const char *const *values, const int lines)
{
  econf_err error;
  econf_ext_value *ext_val;

  if ((error = econf_getExtValue(key_file, group, key, &ext_val)))
  {
    print_error_get (group, key, error);
    econf_freeExtValue(ext_val);
    return false;
  }

  int i=0;
  while (ext_val->values[i] != 0)
  {
    if ((ext_val->values[i] == NULL && values[i] != NULL) ||
	(ext_val->values[i] != NULL && values[i] == NULL) ||
      strcmp(ext_val->values[i], values[i]))
    {
      fprintf (stderr, "ERROR: %s:Expected String:\n'%s'\n, Got:\n'%s'\n",
	       key, values[i], ext_val->values[i]);
      econf_freeExtValue(ext_val);
      return false;
    }
    i++;
  }

  if (i != lines)
  {
    fprintf (stderr,
	     "ERROR: String array does not have expected length: %d exp.: %d\n",
	     i, lines);
    econf_freeExtValue(ext_val);
    return false;    
  }  
  
  econf_freeExtValue(ext_val);
  return true;
}

int
main(void)
{
  econf_file *key_file = NULL;
  econf_err error;
  int retval = 0;

  static const struct {
    const char *const key;
    const char *const val[3];
    const int lines;
  } tests[] = {
    { "string_empty", {""}, 1 },
    { "string_with_spaces", {"string with spaces"}, 1 },
    { "string_escaped_with_leading_and_trailing_spaces", {"string with spaces"}, 1 },
    { "string_with_newlines", {"line one","line two"}, 2 },
    { "string_list_multiple_lines", {"line one","line two"}, 2 },
    { "string_escaped_with_newlines", {"\"line one\n    line two\""}, 1 },
    { "string_with_quotes", {"\\\""}, 1 },
    { "string_with_quotes_v2", {"\\\""}, 1 }
  };
  unsigned int i;  

  error = econf_readFile (&key_file, TESTSDIR"tst-arguments-string/etc/arguments.conf", "=", "#");
  if (error)
  {
    fprintf (stderr, "ERROR: couldn't read configuration file: %s\n", econf_errString(error));
    return 1;
  }

  for (i = 0; i < sizeof(tests)/sizeof(*tests); i++)
  {
    if (!check_StringArray(key_file, "main", tests[i].key, tests[i].val, tests[i].lines))
      retval = 1;
  }

  econf_free(key_file);
  return retval;  
}
