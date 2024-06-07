#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include "libeconf.h"

/* Test case:
   /usr/etc/shells exists
   /usr/etc/shells.d/tcsh exists and contains /bin/csh
   /etc/shells.d/tcsh exists but does not contain /bin/csh

   /bin/csh should not be a valid shell
*/

static int
check_shell(econf_file *key_file, char *shell, int expected)
{
  char *val = NULL;
  econf_err error = econf_getStringValue (key_file, "", shell, &val);

  if (error)
    {
      if (expected)
	{
	  fprintf (stderr, "FAILED: %s: %s\n", shell, econf_errString(error));
	  return 1;
	}
      else
	printf("OK: %s not found\n", shell);
    }
  else
    {
      if (expected)
	printf("OK: %s found\n", shell);
      else
	{
	  fprintf (stderr, "FAILED: %s found\n", shell);
	  return 1;
	}
    }

  return 0;
}

int
main(void)
{
  econf_file *key_file = (econf_file *)-1;
  int retval = 0;
  econf_err error;
  char **keys;
  size_t key_number;

  error = econf_readDirs (&key_file,
				    TESTSDIR"tst-shells2-data/usr/etc",
				    TESTSDIR"tst-shells2-data/etc",
				    "shells", NULL, "", "#");
  if (error)
    {
      fprintf (stderr, "ERROR: econf_readDirs: %s\n",
	       econf_errString(error));
      return 1;
    }

  error = econf_getKeys(key_file, NULL, &key_number, &keys);
  if (error)
    {
      fprintf (stderr, "Error getting all keys: %s\n", econf_errString(error));
      return 1;
    }

  if (key_number == 0)
    {
      fprintf (stderr, "No keys found?\n");
      return 1;
    } else {
      fprintf (stderr, "%ld keys found\n", key_number);
    }
  for (size_t i = 0; i < key_number; i++)
    {
      printf ("%zu: --%s--\n", i, keys[i]);
    }

  if (check_shell(key_file, "/bin/csh", 0) != 0)
    retval = 1;
  if (check_shell(key_file, "/bin/tcsh", 1) != 0)
    retval = 1;
  if (check_shell(key_file, "/bin/foo", 0) != 0)
    retval = 1;

  econf_free (keys);
  econf_free (key_file);

  return retval;
}
