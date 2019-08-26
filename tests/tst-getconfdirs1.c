#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include "libeconf.h"

/* Test case:
   Test the systemd like behavior:
   /usr/etc/getconfdir.conf exists
   /etc/getconfdir.conf exists
   /etc/getconfidr.conf.d/<files>.conf exists

   libeconf should ignore /usr/etc/getconfdir.conf, as this contains
*/

int
check_key(Key_File *key_file, char *key, char *expected_val)
{
  econf_err error = ECONF_SUCCESS;
  const char *val = econf_getStringValue (key_file, "", key, &error);
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
  return 0;
}

int
main(int argc, char **argv)
{
  Key_File *key_file;
  int retval = 0;
  econf_err error;

  key_file = econf_get_conf_from_dirs (
				       TESTSDIR"tst-getconfdirs1-data/usr/etc",
				       TESTSDIR"tst-getconfdirs1-data/etc",
				       "getconfdir", SUFFIX, "=", '#', &error);
  if (key_file == NULL)
    {
      fprintf (stderr, "ERROR: econf_get_conf_from_dirs: %s\n",
	       econf_errString(error));
      return 1;
    }

  if (check_key(key_file, "KEY1", "etcconfd") != 0)
    retval = 1;
  /* XXX fails as we have no way to differentiate between key does not exist and key is empty yet */
  if (check_key(key_file, "USRETC", NULL) != 0)
    retval = 1;
  if (check_key(key_file, "ETC", "true") != 0)
    retval = 1;
  if (check_key(key_file, "OVERRIDE", "true") != 0)
    retval = 1;

  econf_destroy (key_file);

  return retval;
}
