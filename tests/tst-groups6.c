#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include "libeconf.h"

/* Test case:
   Open file and read all group entries and list them.
*/

int
main(void)
{
  econf_file *key_file = (econf_file *)-1;
  char **groups;
  size_t group_number;
  char *val;
  econf_err error;
  int retval = 0;

  if ((error = econf_readFile (&key_file, TESTSDIR"tst-groups6-data/groups.conf", "=", "#")))
    {
      fprintf (stderr, "ERROR: couldn't read configuration file: %s\n", econf_errString(error));
      return 1;
    }

  if ((error = econf_getGroups(key_file, &group_number, &groups)))
    {
      fprintf (stderr, "Error getting all groups: %s\n", econf_errString(error));
      econf_free (key_file);
      return 1;
    }

  if (group_number == 0)
    {
      fprintf (stderr, "No groups found?\n");
      econf_free (key_file);
      return 1;
    }

  if (group_number != 3)
    {
      fprintf (stderr, "Wrong number of groups found, got %zu, expected 3\n",
	       group_number);
      retval = 1;
    }

  printf ("Found the following groups:\n");
  for (size_t i = 0; i < group_number; i++)
    printf ("%zu: %s\n", i, groups[i]);

  /* Try to get the key for each group and check, it is the correct one */
  for (size_t i = 0; i < group_number; i++)
    {
      char **keys;
      size_t key_number;

      error = econf_getKeys (key_file, groups[i], &key_number, &keys);
      if (error)
	{
	  fprintf (stderr, "Error getting all keys for [%s]: %s\n",
		   groups[i], econf_errString(error));
	  retval = 1;
	}
      else
	{
	  for (size_t j = 0; j < key_number; j++)
	    {
	      if ((error = econf_getStringValue (key_file, groups[i], keys[j], &val)))
                  {
                    fprintf (stderr, "Error reading \"%s\" from [%s]: %s\n",
                             keys[j], groups[i], econf_errString(error));
                    retval = 1;
                  }

	      printf ("%zu: Group: %s, Key: %s, Value: %s\n", j, groups[i], keys[j], val);
	      free (val);
	    }
            econf_free (keys);
	}

      if ((error = econf_getStringValue (key_file, groups[i], "key", &val)))
	{
	  fprintf (stderr, "Error getting key for group '%s': %s\n",
		   groups[i], econf_errString(error));
	  retval = 1;
	}
      else
	{
	  printf ("Key: key, Group: %s, Value: %s\n", groups[i], val);
	  free (val);
	}
    }

  econf_free (groups);
  econf_free (key_file);

  return retval;
}
