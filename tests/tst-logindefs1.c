#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include "libeconf.h"

/* Test case:
   Open default login.defs from shadow and try out if we can read all entries
*/

int
main(void)
{
  econf_file *key_file = (econf_file *)-1;
  char **keys;
  size_t key_number;
  char *val;
  econf_err error;

  if ((error = econf_readFile (&key_file, TESTSDIR"tst-logindefs1-data/etc/login.defs", "= \t", "#")))
    {
      fprintf (stderr, "ERROR: couldn't read configuration file: %s\n", econf_errString(error));
      return 1;
    }

  if ((error = econf_getStringValue (key_file, NULL, "USERGROUPS_ENAB", &val)))
    {
      fprintf (stderr, "Error reading USERGROUPS_ENAB: %s\n",
	       econf_errString(error));
      return 1;
    }
  else if (strlen(val) == 0)
    {
      fprintf (stderr, "USERGROUPS_ENAB returns nothing!\n");
      return 1;
    }
  else if (strcmp (val, "yes") != 0)
    {
      fprintf (stderr, "USERGROUPS_ENAB returns wrong value: '%s'\n", val);
      return 1;
    }
  free (val);

  if ((error = econf_getStringValue (key_file, NULL, "ENV_SUPATH", &val)))
    {
      fprintf (stderr, "Error reading ENV_SUPATH: %s\n",
	       econf_errString(error));
      return 1;
    }
  else if (strlen(val) == 0)
    {
      fprintf (stderr, "ENV_SUPATH returns nothing!\n");
      return 1;
    }
  else if (strcmp (val, "PATH=/sbin:/bin:/usr/sbin:/usr/bin") != 0)
    {
      fprintf (stderr, "ENV_SUPATH returns wrong value: '%s'\n", val);
      return 1;
    }
  free (val);

  if ((error = econf_getStringValue (key_file, "", "UMASK", &val)))
    {
      fprintf (stderr, "Error reading UMASK: %s\n",
	       econf_errString(error));
      return 1;
    }
  else if (strlen(val) == 0)
    {
      fprintf (stderr, "UMASK returns nothing!\n");
      return 1;
    }
  else if (strcmp (val, "022") != 0)
    {
      fprintf (stderr, "UMASK returns wrong value: '%s'\n", val);
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
