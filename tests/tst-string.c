#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include "libeconf.h"

/* Test case:
 *  Tests several string format. See tst-arguments-string/etc/arguments.conf.
 */

static int
print_error_get (const char *getgroup, const char *key, econf_err error)
{
  fprintf (stderr, "ERROR: tried to get '%s' from '%s': %s\n",
	   key, getgroup, econf_errString(error));
  return 1;
}

static bool
check_String (econf_file *key_file, const char *group,
	      const char *key, const char *value)
{
  econf_err error;
  char *val_String;

  if ((error = econf_getStringValue(key_file, group, key, &val_String)))
    {
      print_error_get (group, key, error);
      return false;
    }

  if ((value == NULL && val_String != NULL) ||
      (value != NULL && val_String == NULL) ||
      strcmp(val_String, value))
    {
      fprintf (stderr, "ERROR: %s:Expected String:\n'%s'\n, Got:\n'%s'\n",
	       key, value, val_String);
      return false;
    }
  free(val_String);
  return true;
}


int
main(void)
{
  econf_file *key_file = (econf_file *)-1;
  econf_err error;
  int retval = 0;  

  static const struct {
    const char *const key;
    const char *const val;
  } tests[] = {
    { "string_empty", "" },
    { "string_with_spaces", "string with spaces" },
    { "string_escaped_with_leading_and_trailing_spaces", " string with spaces " },
    { "string_with_newlines", "line one\n    line two" },
    { "string_list_multiple_lines", "\n    line one\n    line two" },
    { "string_escaped_with_newlines", "\"line one\n    line two\"" },
    { "string_with_quotes", "\\\"" },
    { "string_with_quotes_v2", "\\\"" }
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
      if (!check_String(key_file, "main", tests[i].key, tests[i].val))
	retval = 1;
    }

  econf_free(key_file);
  return retval;  
}
