#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "libeconf.h"

/* Test case:
   Open default logindefs.data from util-linux and try out if we can read
   all entries
*/

int
main(void)
{
  econf_file *key_file = (econf_file *)-1;
  char **keys;
  size_t key_number;
  econf_err error;

  if ((error = econf_readFile (&key_file, TESTSDIR"tst-logindefs2-data/logindefs.data", "= \t", "#")))
    {
      fprintf (stderr, "ERROR: couldn't read configuration file: %s\n", econf_errString(error));
      return 1;
    }

  if ((error = econf_getKeys(key_file, NULL, &key_number, &keys)))
    {
      fprintf (stderr, "Error getting all keys: %s\n", econf_errString(error));
      econf_free(key_file);
      return 1;
    }
  if (key_number == 0)
    {
      fprintf (stderr, "No keys found?\n");
      econf_free(key_file);
      return 1;
    }
  for (size_t i = 0; i < key_number; i++)
    {
      char *value = NULL;
      econf_getStringValue(key_file, NULL, keys[i], &value);
      printf ("%zu: %s: '%s'\n", i, keys[i], value);
      if (value != NULL)
	free (value);
    }

  int retval = 0;
  char *strval = NULL;
  econf_getStringValue (key_file, NULL, "STRING", &strval);
  if (strval == NULL || strcmp (strval, "this_is_string") != 0)
    {
      fprintf (stderr, "ERROR: %s, expected: %s, got: '%s'\n",
	       "STRING", "this_is_string", strval?strval:"NULL");
      retval = 1;
    }
  if (strval)
    free (strval);

  int intval = 0;
  econf_getIntValue (key_file, NULL, "NUMBER", &intval);
  if (intval == 0 || intval != 123456)
    {
      fprintf (stderr, "ERROR: %s, expected: %i, got: %i\n",
	       "NUMBER", 123456, intval);
      retval = 1;
    }
  error = econf_getIntValue (key_file, NULL, "CRAZY1", &intval);
  if (error != ECONF_VALUE_CONVERSION_ERROR)
    {
      fprintf (stderr, "econf_getIntValue ERROR: expected: %s, got: %s\n",
	       econf_errString(ECONF_VALUE_CONVERSION_ERROR), econf_errString(error));
      retval = 1;
    }

  uint64_t uintval = 0;
  econf_getUInt64Value(key_file, NULL, "OKTAL", &uintval);
  if (uintval == 0 || uintval != 400)
    {
      fprintf (stderr, "ERROR: %s, expected: %i, got: %d\n",
	       "NUMBER", 400, (int) uintval);
      retval = 1;
    }
  error = econf_getUInt64Value(key_file, NULL, "CRAZY1", &uintval);
  if (error != ECONF_VALUE_CONVERSION_ERROR)
    {
      fprintf (stderr, "econf_getUInt64Value ERROR: expected: %s, got: %s\n",
	       econf_errString(ECONF_VALUE_CONVERSION_ERROR), econf_errString(error));
      retval = 1;
    }

  bool boolval = false;
  econf_getBoolValue (key_file, NULL, "BOOLEAN", &boolval);
  if (boolval != true)
    {
      fprintf (stderr, "ERROR: %s, expected: %i, got: %i\n",
	       "BOOLEAN", true, boolval);
      retval = 1;
    }

  strval = NULL;
  econf_getStringValue (key_file, NULL, "CRAZY1", &strval);
  if (strval == NULL || strcmp (strval, "this is crazy format") != 0)
    {
      fprintf (stderr, "ERROR: %s, expected: '%s', got: '%s'\n",
	       "CRAZY1", "this is crazy format", strval?strval:"NULL");
      retval = 1;
    }
  if (strval)
    free (strval);

  strval = NULL;
  econf_getStringValue (key_file, NULL, "CRAZY2", &strval);
  if (strval == NULL || strcmp (strval, "fooBar") != 0)
    {
      fprintf (stderr, "ERROR: %s, expected: '%s', got: '%s'\n",
	       "CRAZY1", "fooBar", strval?strval:"NULL");
      retval = 1;
    }
  if (strval)
    free (strval);

  strval = NULL;
  econf_getStringValue (key_file, NULL, "CRAZY3", &strval);
  if (strval == NULL || strcmp (strval, "FoooBaaar") != 0)
    {
      fprintf (stderr, "ERROR: %s, expected: '%s', got: '%s'\n",
	       "CRAZY3", "FoooBaaar", strval?strval:"NULL");
      retval = 1;
    }
  if (strval)
    free (strval);


  strval = NULL;
  econf_getStringValue (key_file, NULL, "EMPTY", &strval);
  if (strval != NULL)
    {
      fprintf (stderr, "ERROR: %s, expected: 'NULL', got: '%s'\n",
	       "EMPTY", strval?strval:"NULL");
      retval = 1;
    }
  if (strval)
    free (strval);

  strval = NULL;
  econf_getStringValue (key_file, NULL, "END", &strval);
  if (strval == NULL || strcmp (strval, "the is end") != 0)
    {
      fprintf (stderr, "ERROR: %s, expected: '%s', got: '%s'\n",
	       "END", "the is end", strval?strval:"NULL");
      retval = 1;
    }
  if (strval)
    free (strval);

  strval = NULL;
  econf_getStringValue (key_file, NULL, "NOTEXIST", &strval);
  if (strval != NULL)
    {
      fprintf (stderr, "ERROR: %s, expected: '%s', got: '%s'\n",
	       "NOTEXIST", "NULL", strval?strval:"NULL");
      retval = 1;
    }
  if (strval)
    free (strval);

  strval = NULL;
  econf_getStringValue (key_file, NULL, "END", &strval);
  if (strval == NULL || strcmp (strval, "the is end") != 0)
    {
      fprintf (stderr, "ERROR: %s, expected: '%s', got: '%s'\n",
	       "END", "the is end", strval?strval:"NULL");
      retval = 1;
    }
  if (strval)
    free (strval);

  strval = NULL;
  econf_getStringValue (key_file, NULL, "NOTEXIST", &strval);
  if (strval != NULL)
    {
      fprintf (stderr, "ERROR: %s, expected: '%s', got: '%s'\n",
	       "NOTEXIST", "NULL", strval?strval:"NULL");
      retval = 1;
    }
  if (strval)
    free (strval);

  econf_free (keys);
  econf_free (key_file);

  return retval;
}
