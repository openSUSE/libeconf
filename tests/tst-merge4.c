#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include "libeconf.h"

/* Test case:
 * Try to open two files, where one has group entries, the other not
 * Call econf_mergeFiles with this.
 * Application should not crash and return correct key values
*/


static bool
check_string (econf_file *key_file, const char *group, const char *key,
	      const char *value)
{
  econf_err error;

  char *val_String;
  if ((error = econf_getStringValue(key_file, group, key, &val_String)))
    {
      fprintf (stderr, "ERROR: reading (%s,%s): %s\n", group, key, econf_errString(error));
      return false;
    }
  /* NULL means empty string */
  printf ("reading (%s,%s): got '%s', expected '%s'\n", group, key, value, val_String);
  if (strcmp(val_String, value?value:"") != 0)
    {
      fprintf (stderr, "ERROR: reading (%s,%s): expected '%s', got: '%s'\n",
	       group, key, value, val_String);
      return false;
    }
  free(val_String);
  return true;
}

int
main(void)
{
  econf_file *key_file_1 = (econf_file *)-1, *key_file_2 = (econf_file *)-1, *key_file_m = (econf_file *)-1;
  econf_err error;
  int retval = 0;

  error = econf_readFile (&key_file_1, TESTSDIR"tst-merge4-data/data1.conf", "=", "#");
  if (error || key_file_1 == NULL)
    {
      fprintf (stderr, "ERROR: couldn't read data1.conf: %s\n", econf_errString(error));
      return 1;
    }
  error = econf_readFile (&key_file_2, TESTSDIR"tst-merge4-data/data2.conf", " \t=", "#");
  if (error || key_file_2 == NULL)
    {
      fprintf (stderr, "ERROR: coudln't read data2.conf: %s\n", econf_errString(error));
      return 1;
    }

  error = econf_mergeFiles (&key_file_m, key_file_2, key_file_1);
  if (error || key_file_m == NULL)
    {
      fprintf (stderr, "ERROR: error merging configuration files: %s\n", econf_errString(error));
      return 1;
    }

  if (!check_string (key_file_m, "global", "KEY1", "global")) retval = 1;
  if (!check_string (key_file_m, "default", "KEY1", "default")) retval = 1;
  if (!check_string (key_file_m, NULL, "KEY1", "data2")) retval = 1;

  if (key_file_1)
    econf_free (key_file_1);
  if (key_file_2)
    econf_free (key_file_2);
  if (key_file_m)
    econf_free (key_file_m);

  return retval;
}
