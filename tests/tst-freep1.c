#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include "libeconf.h"

/* Test case:
   Use __attribute__((cleanup)) and don't free return values
   yourself.
*/

static int
freeFilep(void)
{
  __attribute__((cleanup(econf_freeFilep))) econf_file *key_file = NULL;
  econf_err error;

  error = econf_readConfig (&key_file,
	                    "foo",
                            "/usr/etc",
			    "bar",
			    "conf", "=", "#");
  if (error != ECONF_NOFILE)
    {
      fprintf (stderr, "ERROR: econf_readConfig: %s\n",
	       econf_errString(error));
      return 1;
    }

  return 0;
}

static int
setValue(econf_file *key_file, const char *group, const char *key, const char *value)
{
  econf_err error;

  if ((error = econf_setStringValue(key_file, group, key, value)))
    {
      fprintf (stderr, "ERROR: couldn't set string value: %s\n",
	       econf_errString(error));
      return 1;
    }
  return 0;
}

static int
freeArrayp(void)
{
  __attribute__((cleanup(econf_freeFilep))) econf_file *key_file = NULL;
  econf_err error;

  if ((error = econf_newKeyFile(&key_file, '=', '#')))
    {
      fprintf (stderr, "ERROR: couldn't create new file: %s\n",
               econf_errString(error));
      return 1;
    }

  if (setValue(key_file, "Group1", "Key1", "value1")) return 1;
  if (setValue(key_file, "Group1", "Key2", "value2")) return 1;
  if (setValue(key_file, "Group1", "Key3", "value3")) return 1;
  if (setValue(key_file, "Group2", "Key1", "value1")) return 1;
  if (setValue(key_file, "Group3", "Key1", "value1")) return 1;
  if (setValue(key_file, "Group4", "Key4", "value4")) return 1;

  __attribute__((cleanup(econf_freeArrayp))) char **groups;
  size_t group_number;
  if ((error = econf_getGroups(key_file, &group_number, &groups)))
    {
      fprintf (stderr, "Error getting all groups: %s\n", econf_errString(error));
      return 1;
    }

  __attribute__((cleanup(econf_freeArrayp))) char **keys;
  size_t key_number;
  error = econf_getKeys (key_file, "Group1", &key_number, &keys);
  if (error)
    {
      fprintf (stderr, "Error getting all keys for [%s]: %s\n",
	       "Group1", econf_errString(error));
      return 1;
    }

  return 0;
}


int
main(void)
{
  if (freeFilep() != 0) return 1;
  if (freeArrayp() != 0) return 1;

  return 0;
}
