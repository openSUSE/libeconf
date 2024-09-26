#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include "libeconf.h"

/* Test case:
   Open file and read all group entries and list them.
*/

static int
check_key(econf_file *key_file, char *group, char *key, char *expected_val)
{
  char *val = NULL;
  econf_err error = econf_getStringValue (key_file, group, key, &val);

  if (expected_val == NULL)
    {
      if (val == NULL)
	return 0;

      fprintf (stderr, "ERROR: %s/%s has value \"%s\"\n", group, key, val);
      return 1;
    }
  if (val == NULL || strlen(val) == 0)
    {
      fprintf (stderr, "ERROR: %s/%s returns nothing! (%s)\n", group, key,
	       econf_errString(error));
      return 1;
    }
  if (strcmp (val, expected_val) != 0)
    {
      fprintf (stderr, "ERROR: %s/%s is not \"%s\"\n", group, key, expected_val);
      return 1;
    }

  printf("Ok: %s/%s=%s\n", group, key, val);
  free (val);
  return 0;
}

int
main(void)
{
  econf_file *key_file = (econf_file *)-1;
  char **groups;
  size_t group_number;
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
	  if (!strcmp("section1", groups[i]) && key_number != 3 ) {
	    fprintf (stderr, "Wrong number of keys found for group %s, got %zu, expected 3\n",
		     groups[i], key_number);
	    retval = 1;
	  }
	  if ((!strcmp("section1", groups[i]) || !strcmp("section1", groups[i])) && key_number != 3 ) {
	    fprintf (stderr, "Wrong number of keys found for group %s, got %zu, expected 1\n",
		     groups[i], key_number);
	    retval = 1;
	  }
	}
      econf_freeArray(keys);
    }
  if (check_key(key_file, "section1", "key", "yes") != 0 ||
      check_key(key_file, "section1", "key1", "no") != 0 ||
      check_key(key_file, "section1", "key2", "yes") != 0 ||
      check_key(key_file, "section2", "key", "yes") != 0 ||
      check_key(key_file, "section3", "key", "yes"))
	  return 1;

  econf_free (groups);
  econf_free (key_file);

  return retval;
}
