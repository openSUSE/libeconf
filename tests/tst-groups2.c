#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include "libeconf.h"

/* Test case:
   Create internal econf_file with one group and key, check it's valid,
   list all groups, which should return exactly this one.
*/

int
main(void)
{
  econf_file *key_file = (econf_file *)-1;
  char **groups;
  size_t group_number;
  const char *group = "towel";
  const char *key = "42";
  const char *val = "The answer, not question";
  econf_err error;
  int retval = 0;

  if ((error = econf_newKeyFile (&key_file, '=', '#')))
    {
      fprintf (stderr, "ERROR: couldn't read configuration file: %s\n",
	       econf_errString(error));
      return 1;
    }

  if ((error = econf_setStringValue(key_file, group, key, val)))
    {
      fprintf (stderr, "Error setting key '%s' with value '%s' in group '%s': %s\n",
	       key, val, group, econf_errString(error));
      return 1;
    }

  char *gval;
  if ((error = econf_getStringValue(key_file, group, key, &gval)))
    {
      fprintf (stderr, "Error getting keys: %s\n", econf_errString(error));
      return 1;
    }

  if (strcmp(val, gval) != 0)
    {
      fprintf (stderr, "Wrong value: expected='%s', got='%s'\n",
	       val, gval);
      return 1;
    }
  free (gval);

  if ((error = econf_getGroups(key_file, &group_number, &groups)))
    {
      fprintf (stderr, "Error getting all groups: %s\n", econf_errString(error));
      return 1;
    }
  if (group_number == 0)
    {
      fprintf (stderr, "No groups found?\n");
      return 1;
    }
  if (group_number > 1)
    {
      fprintf (stderr, "Too many groups found? (got %zu, expected 1)\n", group_number);
      retval = 1;
    }
  for (size_t i = 0; i < group_number; i++)
    {
      printf ("%zu: %s\n", i, groups[i]);
    }

  econf_free (groups);
  econf_free (key_file);

  return retval;
}
