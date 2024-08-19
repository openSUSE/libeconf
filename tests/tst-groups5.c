#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include "libeconf.h"

/* Test case:
   Open file with one empty group entry and list it.
*/

int
main(void)
{
  econf_file *key_file = (econf_file *)-1;
  char **groups;
  size_t group_number;
  econf_err error;
  int retval = 0;

  if ((error = econf_readFile (&key_file, TESTSDIR"tst-groups5-data/groups.conf", "=", "#")))
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
  if (group_number != 1)
    {
      fprintf (stderr, "Wrong number of groups found, got %zu, expected 1\n",
	       group_number);
      retval = 1;
    }
  printf ("Found the following groups:\n");
  for (size_t i = 0; i < group_number; i++)
    printf ("%zu: %s\n", i, groups[i]);

  econf_free (groups);
  econf_free (key_file);

  return retval;
}
