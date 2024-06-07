#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include "libeconf.h"

/* Test case:
   Provide some NULL pointers as arguments. libeconf shouldn't
   crash but do a correct error handling
*/

int
main(void)
{
  econf_file *key_file = (econf_file *)-1;
  char *val;
  econf_err error;

  error = econf_readFile (&key_file, TESTSDIR"tst-arguments-data/etc/arguments.conf", "=", "#");
  if (error)
    {
      fprintf (stderr, "ERROR: couldn't read configuration file: %s\n", econf_errString(error));
      return 1;
    }

  if ((error = econf_getStringValue (key_file, "", "KEY", &val)) || val == NULL || strlen(val) == 0)
    {
      fprintf (stderr, "KEY returns nothing!\n");
      return 1;
    }
  free (val);

  error = econf_getStringValue (key_file, "", "NOKEY", &val);
  if (error != ECONF_NOKEY)
    {
      fprintf (stderr, "Should return ECONF_NOKEY. Returned: %s\n", econf_errString(error));
      return 1;
    }

  error = econf_getStringValue (key_file, "", "KEY", NULL);
  if (error != ECONF_ARGUMENT_IS_NULL_VALUE)
    {
      fprintf (stderr, "Should return ECONF_ARGUMENT_IS_NULL_VALUE. Returned: %s\n",
	       econf_errString(error));
      return 1;
    }

  econf_free (key_file);

  return 0;
}
