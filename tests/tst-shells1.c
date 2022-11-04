#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include "libeconf.h"

/* Test case:
   Open default /etc/shells and try out if we can parse entries
*/

int
main(void)
{
  econf_file *key_file = NULL;
  char **keys;
  size_t key_number;
  char *val;
  econf_err error;

  if ((error = econf_readFile (&key_file, TESTSDIR"tst-shells1-data/etc/shells", "", "#")))
    {
      fprintf (stderr, "ERROR: couldn't read configuration file: %s\n", econf_errString(error));
      return 1;
    }

  if ((error = econf_getStringValue (key_file, NULL, "/usr/bin/bash", &val)))
    {
      fprintf (stderr, "Error reading /usr/bin/bash: %s\n",
	       econf_errString(error));
      return 1;
    }
  else if (val != NULL && strlen(val) > 0)
    {
      fprintf (stderr, "/usr/bin/bash returns wrong value: '%s'\n", val);
      return 1;
    }
  free (val);

  if (!(error = econf_getStringValue (key_file, NULL, "doesnotexist", &val)))
    {
      fprintf (stderr, "No error looking for \"doesnotexist\"!\n");
      return 1;
    }
  free (val);

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
    }
  for (size_t i = 0; i < key_number; i++)
    {
      printf ("%zu: %s\n", i, keys[i]);
    }

  econf_free (keys);
  econf_free (key_file);

  return 0;
}
