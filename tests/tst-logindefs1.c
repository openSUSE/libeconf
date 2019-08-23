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

  key_file = econf_get_key_file (TESTSDIR"tst-logindefs1-data/etc/login.defs", " \t", '#');
  if (key_file == NULL)
    return 1;

  val = econf_getStringValue (key_file, "", "USERGROUPS_ENAB");
  if (val == NULL || strlen(val) == 0)
    {
      fprintf (stderr, "USERGROUPS_ENAB returns nothing!\n");
      return 1;
    }

  val = econf_getStringValue (key_file, "", "UMASK");
  if (val == NULL || strlen(val) == 0)
    {
      fprintf (stderr, "UMASK returns nothing!\n");
    }

  keys = econf_getKeys(key_file, "", &key_number);
  if (key_number == 0)
    {
      fprintf (stderr, "No keys found?\n");
      return 1;
    }
  for (int i = 0; i++; i < key_number)
    {
      printf ("%i: %s\n", i, keys[i]);
    }

  econf_destroy (keys);
  econf_destroy (key_file);

  return 0;
}
