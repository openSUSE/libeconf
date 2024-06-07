#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include "libeconf.h"

/* Test case:
   Read quoted strings from conf file
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
      fprintf (stderr, "ERROR: Expected String: '%s', Got: '%s'\n",
	       value, val_String);
      return false;
    }
  free(val_String);
  return true;
}


int
main(void)
{
  econf_file *key_file = (econf_file *)-1;
  const char *group="quote";
  econf_err error;
  int retval = 0;
  static const struct {
    const char *const key;
    const char *const val;
  } tests[] = {
    { "unquoted", "unquoted" },
    { "spaces_1", "spaces 1" },
    { "spaces_2", "  spaces 2  " },
    { "mid_quote", "mid\"quote" },
    { "begin_quote", "\"begin_quote" },
    { "end_quote", "end quote\"", },
    { "quoted_word", "a \"quoted\" word", },
    { "quoted_word1", "a 'quoted' word", },
    { "quoted_string", "\"a quoted string\"", },
    { "quoted_string1", "'a quoted string'", },
    { "unbal1", "\"unbalanced", },
    { "unbal2", "unbalanced\"", },
    { "unbal3", "\"not \"well\" balanced", },
    { "unbal4", "not \"well\" balanced\"", },
    { "quoted_delim", "a=b", },
    { "quoted_delim1", "=>b", },
    { "unquoted_delim", "a=b", },
    { "unquoted_delim1", "=>b", },
    { "unquoted_delim2", "a=b", },
    { "unquoted_delim3", "=>b", },
  };
  unsigned int i;

  if ((error = econf_readFile (&key_file, TESTSDIR"tst-quote1-data/quote.conf", "=", "#")))
    {
      fprintf (stderr, "ERROR: couldn't read configuration file: %s\n", econf_errString(error));
      return 1;
    }

  for (i = 0; i < sizeof(tests)/sizeof(*tests); i++)
    {
      if (!check_String(key_file, group, tests[i].key, tests[i].val))
	retval = 1;
    }

  econf_free(key_file);
  return retval;
}
