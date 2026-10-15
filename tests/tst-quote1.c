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
  econf_file *key_file = NULL;
  const char *group="quote";
  econf_err error;
  int retval = 0;
  static const struct {
    const char *const key;
    const char *const val;
    const bool quotes;
  } tests[] = {
    { "unquoted", "unquoted", false },
    { "spaces_1", "spaces 1", false },
    { "spaces_2", "  spaces 2  ", true },
    { "mid_quote", "mid\"quote", false },
    { "begin_quote", "\"begin_quote", true },
    { "end_quote", "end quote\"", true },
    { "quoted_word", "a \"quoted\" word", true },
    { "quoted_word1", "a 'quoted' word", true },
    { "quoted_string", "\"a quoted string\"", true },
    { "quoted_string1", "'a quoted string'", true },
    { "unbal1", "\"unbalanced", true },
    { "unbal2", "unbalanced\"", false },
    { "unbal3", "\"not \"well\" balanced", true },
    { "unbal4", "not \"well\" balanced\"", false },
    { "quoted_delim", "a=b", true },
    { "quoted_delim1", "=>b", true },
    { "unquoted_delim", "a=b", false },
    { "unquoted_delim1", "=>b", false },
    { "unquoted_delim2", "a=b", false },
    { "unquoted_delim3", "=>b", false },
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

      if (econf_hasQuotes(key_file, group, tests[i].key) != tests[i].quotes)
	{
	  fprintf (stderr, "ERROR: econf_hasQuotes('%s') expected %d\n",
		   tests[i].key, tests[i].quotes);
	  retval = 1;
	}
    }

  if (econf_hasQuotes(key_file, group, "does_not_exist"))
    {
      fprintf (stderr, "ERROR: econf_hasQuotes for unknown key should be false\n");
      retval = 1;
    }

  if (econf_hasQuotes(NULL, group, "unquoted"))
    {
      fprintf (stderr, "ERROR: econf_hasQuotes with NULL key_file should be false\n");
      retval = 1;
    }

  if ((error = econf_setQuotes(key_file, group, "unquoted", true)))
    {
      fprintf (stderr, "ERROR: econf_setQuotes('unquoted', true) failed: %s\n",
	       econf_errString(error));
      retval = 1;
    }
  else if (!econf_hasQuotes(key_file, group, "unquoted"))
    {
      fprintf (stderr, "ERROR: econf_hasQuotes('unquoted') expected true after econf_setQuotes\n");
      retval = 1;
    }

  if ((error = econf_setQuotes(key_file, group, "spaces_2", false)))
    {
      fprintf (stderr, "ERROR: econf_setQuotes('spaces_2', false) failed: %s\n",
	       econf_errString(error));
      retval = 1;
    }
  else if (econf_hasQuotes(key_file, group, "spaces_2"))
    {
      fprintf (stderr, "ERROR: econf_hasQuotes('spaces_2') expected false after econf_setQuotes\n");
      retval = 1;
    }

  if (econf_setQuotes(key_file, group, "does_not_exist", true) != ECONF_NOKEY)
    {
      fprintf (stderr, "ERROR: econf_setQuotes for unknown key should return ECONF_NOKEY\n");
      retval = 1;
    }

  if (econf_setQuotes(NULL, group, "unquoted", true) != ECONF_FILE_LIST_IS_NULL)
    {
      fprintf (stderr, "ERROR: econf_setQuotes with NULL key_file should return ECONF_FILE_LIST_IS_NULL\n");
      retval = 1;
    }

  econf_free(key_file);
  return retval;
}
