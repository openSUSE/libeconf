#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <values.h>

#include "libeconf.h"

/* Test case:
   Create new file in memory, store all kind of types and read them again
   to verify, they where correctly stored.
*/

void
exit_with_error_set (const char *type, econf_err error)
{
  fprintf (stderr, "ERROR: couldn't set type '%s': %s\n",
	   type, econf_errString(error));
  exit (1);
}

void
exit_with_error_get (const char *type, econf_err error)
{
  fprintf (stderr, "ERROR: couldn't get type '%s': %s\n",
	   type, econf_errString(error));
  exit (1);
}

/* The econf_get*Value functions are identical except for return
   type, so let's create them via a macro. */
#define check_type(TYPE, FCT_TYPE, FCT_TYPE_STR, PR)	 \
bool check_ ## FCT_TYPE (Key_File *key_file, TYPE value) \
{ \
   econf_err error; \
\
  if (!econf_set ## FCT_TYPE ## Value(key_file, NULL, "KEY", value, &error)) \
    exit_with_error_set (FCT_TYPE_STR, error); \
  TYPE val_ ## FCT_TYPE = econf_get ## FCT_TYPE ## Value(key_file, NULL, "KEY", &error); \
  if (error) \
    exit_with_error_get (FCT_TYPE_STR, error); \
  if (val_ ## FCT_TYPE != value) \
    { \
      fprintf (stderr, "ERROR: Set "FCT_TYPE_STR": '"PR"', Got: '"PR"'\n", value, val_ ## FCT_TYPE); \
      return false; \
    } \
  return true; \
}

check_type(int32_t, Int, "Int", "%i")
check_type(uint32_t, UInt, "UInt", "%ui")
check_type(int64_t, Int64, "Int64", "%li")
check_type(uint64_t, UInt64, "UInt64", "%lu")
check_type(float, Float, "Float", "%f")
check_type(double, Double, "Double", "%f")

/* check_type(const char *, String, "String", "%s") */
bool check_String (Key_File *key_file, const char *value)
{
  econf_err error;

  if (!econf_setStringValue(key_file, NULL, "KEY", value, &error))
    exit_with_error_set ("String", error);

  const char *val_String = econf_getStringValue(key_file, NULL, "KEY", &error);
  if (error)
    exit_with_error_get ("String", error);
  /* NULL means empty string */
  if (strcmp(val_String, value?value:"") != 0)
    {
      fprintf (stderr, "ERROR: Set String: '%s', Got: '%s'\n", value, val_String);
      return false;
    }
  return true;
}


/* check_type(bool, Bool, "Bool", "%s") */
bool check_Bool (Key_File *key_file, const char *value, bool expect)
{
  econf_err error;

  if (!econf_setBoolValue(key_file, NULL, "KEY", value, &error))
    exit_with_error_set ("Bool", error);

  bool val_Bool = econf_getBoolValue(key_file, NULL, "KEY", &error);
  if (error)
    exit_with_error_get ("Bool", error);
  if (val_Bool != expect)
    {
      fprintf (stderr, "ERROR: Set Bool: '%s' (%i), Got: '%i'\n", value, expect, val_Bool);
      return false;
    }
  return true;
}


int
main(int argc, char **argv)
{
  Key_File *key_file;
  econf_err error;
  int retval = 0;

  key_file = econf_newKeyFile('=', '#', &error);
  if (key_file == NULL)
    {
      fprintf (stderr, "ERROR: couldn't create new file: %s\n",
	       econf_errString(error));
      return 1;
    }

  /* test reading a key from an empty key_file */
  econf_getIntValue(key_file, NULL, "KEY", &error);
  if (error != ECONF_NOKEY)
    exit_with_error_get ("int", error);

  if (!check_Int (key_file, INT32_MAX)) retval=1;
  if (!check_Int (key_file, INT32_MIN)) retval=1;
  if (!check_UInt (key_file, UINT32_MAX)) retval=1;
  if (!check_UInt (key_file, 0)) retval=1;

  if (!check_Int64 (key_file, INT64_MAX)) retval=1;
  if (!check_Int64 (key_file, INT64_MIN)) retval=1;
  if (!check_UInt64 (key_file, UINT64_MAX)) retval=1;
  if (!check_UInt64 (key_file, 0)) retval=1;

  if (!check_Float (key_file, FLT_MAX)) retval=1;
  if (!check_Float (key_file, FLT_MIN)) retval=1;

  if (!check_Double (key_file, DBL_MAX)) retval=1;
  if (!check_Double (key_file, DBL_MIN)) retval=1;

  if (!check_String (key_file, NULL)) retval=1;
  if (!check_String (key_file, "")) retval=1;
  if (!check_String (key_file, " ")) retval=1;
  if (!check_String (key_file, "This should become a long and complicated string, but up to now it is not = ! $ % € ß")) retval=1;

  if (!check_Bool (key_file, "True", true)) retval=1;
  if (!check_Bool (key_file, "true", true)) retval=1;
  if (!check_Bool (key_file, "Yes", true)) retval=1;
  if (!check_Bool (key_file, "yes", true)) retval=1;
  if (!check_Bool (key_file, "1", true)) retval=1;
  if (!check_Bool (key_file, "False", false)) retval=1;
  if (!check_Bool (key_file, "false", false)) retval=1;
  if (!check_Bool (key_file, "No", false)) retval=1;
  if (!check_Bool (key_file, "no", false)) retval=1;
  if (!check_Bool (key_file, "0", false)) retval=1;

  econf_destroy (key_file);

  return retval;
}
