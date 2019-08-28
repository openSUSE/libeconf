#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include "libeconf.h"

/* Test case:
   Test the systemd like behavior:
   /usr/etc/getconfdir.conf exists
   No configuration files in /etc exists

   libeconf should only use /usr/etc/getconfdir.conf
*/

int
check_key(Key_File *key_file, char *key, char *expected_val)
{
  char *val = NULL;
  econf_err error = econf_getStringValue (key_file, "", key, &val);
  if (expected_val == NULL)
    {
      if (val == NULL)
        return 0;

      fprintf (stderr, "ERROR: %s has value \"%s\"\n", key, val);
      return 1;
    }
  if (val == NULL || strlen(val) == 0)
    {
      fprintf (stderr, "ERROR: %s returns nothing! (%s)\n", key,
               econf_errString(error));
      return 1;
    }
  if (strcmp (val, expected_val) != 0)
    {
      fprintf (stderr, "ERROR: %s is not \"%s\"\n", key, expected_val);
      return 1;
    }

  printf("Ok: %s=%s\n", key, val);
  free (val);
  return 0;
}

int
main(int argc, char **argv)
{
  Key_File *key_file;
  int retval = 0;
  econf_err error;

  key_file = econf_get_conf_from_dirs (
				       TESTSDIR"tst-getconfdirs3-data/usr/etc",
				       TESTSDIR"tst-getconfdirs3-data/etc",
				       "getconfdir", ".conf", "=", '#', &error);
  if (key_file == NULL)
    {
      fprintf (stderr, "ERROR: econf_get_conf_from_dirs returned NULL: %s\n",
	       econf_errString(error));
      return 1;
    }

  if (check_key(key_file, "KEY1", "usretc") != 0)
    retval = 1;
  if (check_key(key_file, "USRETC", "true") != 0)
    retval = 1;
  if (check_key(key_file, "ETC", NULL) != 0)
    retval = 1;
  if (check_key(key_file, "OVERRIDE", NULL) != 0)
    retval = 1;

  econf_destroy (key_file);

  return retval;
}
