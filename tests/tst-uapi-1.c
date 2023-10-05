#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include "libeconf.h"
#include "../lib/helpers.h"
#include "../lib/mergefiles.h"
#include "../lib/readconfig.h"

/* Test case:
   Test the systemd like behavior:
   /usr/foo/etc/getconfdir.conf exists
   /run/foo/getconfdir.conf exists
   /etc/foo/getconfidr.conf.d/<files>.conf exists

   libeconf should ignore /usr/foo/etc/getconfdir.conf, as this contains
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
  econf_file *key_file = NULL;
  int retval = 0;
  econf_err error;

  error = readConfigWithCallback (&key_file,
				  TESTSDIR"tst-uapi-1-data/usr/etc/foo",
				  TESTSDIR"tst-uapi-1-data/run/foo",
				  TESTSDIR"tst-uapi-1-data/etc/foo",   
				  "getconfdir", "conf", "=", "#",
				  NULL, 0, NULL, NULL);
  if (error)
    {
      fprintf (stderr, "ERROR: econf_readConfig: %s\n",
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
