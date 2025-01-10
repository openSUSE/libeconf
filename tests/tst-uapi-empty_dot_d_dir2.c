#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include "libeconf.h"
bool callback_without_error(const char *filename, const void *data);
bool callback_with_error(const char *filename, const void *data);

/* Test case:

   /etc/environment which is empty
   /usr/etc/environment.d/a.conf which is empty
   /usr/etc/environment.d/b.conf which is empty
   /usr/etc/environment.d/c.conf which is NOT empty

   libeconf should read these files and return one entry.
*/

static int
check_key(econf_file *key_file, char *key, char *expected_val)
{
  char *val = NULL;
  econf_err error = econf_getStringValue (key_file, "", key, &val);
  if (expected_val == NULL)
    {
      if (val == NULL)
	return 0;

      fprintf (stderr, "ERROR: %s has value \"%s\"\n", key, val);
      return 1;
    }
  if (val == NULL || strlen(val) == 0)
    {
      fprintf (stderr, "ERROR: %s returns nothing! (%s)\n", key,
	       econf_errString(error));
      return 1;
    }
  if (strcmp (val, expected_val) != 0)
    {
      fprintf (stderr, "ERROR: %s is not \"%s\"\n", key, expected_val);
      return 1;
    }

  printf("Ok: %s=%s\n", key, val);
  free (val);
  return 0;
}

int
main(void)
{
  econf_file *key_file = NULL;
  int retval = 0;
  econf_err error;

  if ((error = econf_newKeyFile_with_options(&key_file, "ROOT_PREFIX="TESTSDIR)))
    {
      fprintf (stderr, "ERROR: couldn't allocate new file: %s\n",
	       econf_errString(error));
      return 1;
    }
  
  error = econf_readConfig (&key_file,
	                    NULL,
                            "/usr/etc",
			    "environment",
			    "", "=", "#");

  if (error)
    {
      fprintf (stderr, "ERROR: econf_readConfig: %s\n",
	       econf_errString(error));
      return 1;
    }

  if (check_key(key_file, "Y2STYLE", "dark.qss") != 0)
    retval = 1;

  econf_free (key_file);

  return retval;
}
