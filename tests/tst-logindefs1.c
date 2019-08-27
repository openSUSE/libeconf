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
main(int argc, char **argv)
{
  Key_File *key_file;
  char **keys;
  size_t key_number;
  char *val;
  econf_err error;

  key_file = econf_get_key_file (TESTSDIR"tst-logindefs1-data/etc/login.defs", " \t", '#', &error);
  if (key_file == NULL)
    {
      fprintf (stderr, "ERROR: couldn't read configuration file: %s\n", econf_errString(error));
      return 1;
    }

  val = econf_getStringValue (key_file, NULL, "USERGROUPS_ENAB", &error);
  if (val == NULL)
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

  val = econf_getStringValue (key_file, "", "UMASK", &error);
  if (val == NULL)
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

  keys = econf_getKeys(key_file, NULL, &key_number, &error);
  if (keys == NULL && error)
    {
      fprintf (stderr, "Error getting all keys: %s\n", econf_errString(error));
      return 1;
    }
  if (key_number == 0)
    {
      fprintf (stderr, "No keys found?\n");
      return 1;
    }
  for (int i = 0; i < key_number; i++)
    {
      printf ("%i: %s\n", i, keys[i]);
    }

  econf_destroy (keys);
  econf_destroy (key_file);

  return 0;
}
