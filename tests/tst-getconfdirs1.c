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

static int
check_key(econf_file *key_file, char *key, char *expected_val)
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
main(void)
{
  econf_file *key_file = (econf_file *)-1;
  int retval = 0;
  econf_err error;

  error = econf_readDirs (&key_file,
				    TESTSDIR"tst-getconfdirs1-data/usr/etc",
				    TESTSDIR"tst-getconfdirs1-data/etc",
				    "getconfdir", SUFFIX, "=", "#");
  if (error)
    {
      fprintf (stderr, "ERROR: econf_readDirs: %s\n",
	       econf_errString(error));
      return 1;
    }

  if (check_key(key_file, "KEY1", "etcconfd") != 0)
    retval = 1;
  if (check_key(key_file, "USRETC", NULL) != 0)
    retval = 1;
  if (check_key(key_file, "ETC", "true") != 0)
    retval = 1;
  if (check_key(key_file, "OVERRIDE", "true") != 0)
    retval = 1;

  econf_free (key_file);

  return retval;
}
