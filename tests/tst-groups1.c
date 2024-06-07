#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <float.h>

#include "libeconf.h"

/* Test case:
   Create new file in memory, store all kind of types and read them again
   to verify, they where correctly stored.
*/

static int
print_error_set (const char *type, econf_err error)
{
  fprintf (stderr, "ERROR: couldn't set type '%s': %s\n",
	   type, econf_errString(error));
  return 1;
}

static int
print_error_get (const char *value, const char *setgroup,
                 const char *getgroup, econf_err error)
{
  fprintf (stderr, "ERROR: set '%s' in group '%s', tried to get from '%s': %s\n",
	   value, setgroup, getgroup, econf_errString(error));
  return 1;
}

/* check_type(const char *, String, "String", "%s") */
static bool
check_String (econf_file *key_file, const char *value,
              const char *setgroup, const char *getgroup)
{
  econf_err error;

  if ((error = econf_setStringValue(key_file, setgroup, "KEY", value)))
    {
      print_error_set ("String", error);
      return false;
    }

  char *val_String;
  if ((error = econf_getStringValue(key_file, getgroup, "KEY", &val_String)))
    {
      print_error_get (value, setgroup, getgroup, error);
      return false;
    }
  /* NULL means empty string */
  if (strcmp(val_String, value?value:"") != 0)
    {
      fprintf (stderr, "ERROR: Set String: '%s', Got: '%s'\n", value, val_String);
      return false;
    }
  free(val_String);
  return true;
}


int
main(void)
{
  econf_file *key_file = (econf_file *)-1;
  econf_err error;
  int retval = 0;

  if ((error = econf_newKeyFile(&key_file, '=', '#')))
    {
      fprintf (stderr, "ERROR: couldn't create new file: %s\n",
	       econf_errString(error));
      return 1;
    }

  /* test reading a key from an empty key_file */
  int dummy;
  if ((error = econf_getIntValue(key_file, "doesnotexist", "KEY", &dummy)) != ECONF_NOKEY)
    {
      print_error_get ("int", "doesnotexist", "doesnotexist", error);
      retval = 1;
    }

  if (!check_String (key_file, NULL, NULL, NULL)) retval=1;
  if (!check_String (key_file, NULL, "group1", "[group1]")) retval=1;
  if (!check_String (key_file, "", "group2", "[group2]")) retval=1;
  if (!check_String (key_file, "TaTa", "[group2]", "group2")) retval=1;
  if (!check_String (key_file, "TeST", "[group3]", "[group3]")) retval=1;
  if (!check_String (key_file, "dummy", "group4", "group4")) retval=1;

  econf_free (key_file);

  return retval;
}
