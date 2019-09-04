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
  econf_file *key_file = NULL;
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

  econf_free (key_file);

  return 0;
}
