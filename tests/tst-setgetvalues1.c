#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <float.h>
#include <inttypes.h>

#include "libeconf.h"

/* Test case:
   Create new file in memory, store all kind of types and read them again
   to verify, they where correctly stored.
*/

static void
exit_with_error_set (const char *type, econf_err error)
{
  fprintf (stderr, "ERROR: couldn't set type '%s': %s\n",
	   type, econf_errString(error));
  exit (1);
}

static void
exit_with_error_get (const char *type, econf_err error)
{
  fprintf (stderr, "ERROR: couldn't get type '%s': %s\n",
	   type, econf_errString(error));
  exit (1);
}

/* The econf_get*Value functions are identical except for return
   type, so let's create them via a macro. */
#define check_type(TYPE, FCT_TYPE, FCT_TYPE_STR, PR)	 \
static bool check_ ## FCT_TYPE (econf_file *key_file, TYPE value) \
{ \
   econf_err error; \
\
   if ((error = econf_set ## FCT_TYPE ## Value(key_file, NULL, "KEY", value))) \
    exit_with_error_set (FCT_TYPE_STR, error); \
  TYPE val_ ## FCT_TYPE; \
  if ((error = econf_get ## FCT_TYPE ## Value(key_file, NULL, "KEY", &val_ ## FCT_TYPE))) \
    exit_with_error_get (FCT_TYPE_STR, error); \
  if (val_ ## FCT_TYPE != value) \
    { \
      fprintf (stderr, "ERROR: Set "FCT_TYPE_STR": '%"PR"', Got: '%"PR"'\n", value, val_ ## FCT_TYPE); \
      return false; \
    } \
  return true; \
}

check_type(int32_t, Int, "Int", PRId32)
check_type(uint32_t, UInt, "UInt", PRIu32)
check_type(int64_t, Int64, "Int64", PRId64)
check_type(uint64_t, UInt64, "UInt64", PRIu64)
check_type(float, Float, "Float", "f")
check_type(double, Double, "Double", "f")

/* check_type(const char *, String, "String", "%s") */
static bool
check_String (econf_file *key_file, const char *value)
{
  econf_err error;

  if ((error = econf_setStringValue(key_file, NULL, "KEY", value)))
    exit_with_error_set ("String", error);

  char *val_String;
  if ((error = econf_getStringValue(key_file, NULL, "KEY", &val_String)))
    exit_with_error_get ("String", error);
  /* NULL means empty string */
  if (strcmp(val_String, value?value:"") != 0)
    {
      fprintf (stderr, "ERROR: Set String: '%s', Got: '%s'\n", value, val_String);
      return false;
    }
  free(val_String);
  return true;
}


/* check_type(bool, Bool, "Bool", "%s") */
static bool
check_Bool (econf_file *key_file, const char *value, bool expect)
{
  econf_err error;

  if ((error = econf_setBoolValue(key_file, NULL, "KEY", value)))
    exit_with_error_set ("Bool", error);

  bool val_Bool;
  if ((error = econf_getBoolValue(key_file, NULL, "KEY", &val_Bool)))
    exit_with_error_get ("Bool", error);
  if (val_Bool != expect)
    {
      fprintf (stderr, "ERROR: Set Bool: '%s' (%i), Got: '%i'\n", value, expect, val_Bool);
      return false;
    }
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
  if ((error = econf_getIntValue(key_file, NULL, "KEY", &dummy)) != ECONF_NOKEY)
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
  /* setting and getting NULL values */
  if ((error = econf_setBoolValue(key_file, NULL, "KEY", NULL) != ECONF_SUCCESS))
    exit_with_error_set ("boolean", error);
  bool val_Bool;
  if ((error = econf_getBoolValue(key_file, NULL, "KEY", &val_Bool) != ECONF_KEY_HAS_NULL_VALUE))
    exit_with_error_get ("boolean", error);
  if ((error = econf_setBoolValue(key_file, NULL, "KEY", "") != ECONF_SUCCESS))
    exit_with_error_set ("boolean", error);
  if ((error = econf_getBoolValue(key_file, NULL, "KEY", &val_Bool) != ECONF_KEY_HAS_NULL_VALUE))
    exit_with_error_get ("boolean", error);  

  /* Error Handling */
  if ((error = econf_setIntValue (NULL, NULL, "test", 8)) != ECONF_FILE_LIST_IS_NULL)
    exit_with_error_set ("int", error);
  if ((error = econf_setIntValue (key_file, NULL, NULL, 8)) != ECONF_EMPTYKEY)
    exit_with_error_set ("int", error);
  if ((error = econf_setIntValue (key_file, NULL, "", 8)) != ECONF_EMPTYKEY)
    exit_with_error_set ("int", error);
  if ((error = econf_setBoolValue(key_file, NULL, "KEY", "foo")) != ECONF_WRONG_BOOLEAN_VALUE)
    exit_with_error_set ("Bool", error);

  econf_free (key_file);

  return retval;
}
