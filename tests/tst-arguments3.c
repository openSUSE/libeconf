#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include "libeconf.h"

/* Test case:
 * Provide a NULL pointer for the key. We should return an error, not crash
*/

int
main(void)
{
  econf_file *key_file = (econf_file *)-1;
  char *val;
  econf_err error;

  if ((error = econf_readFile (&key_file, TESTSDIR"tst-arguments-data/etc/arguments.conf", "=", "#")))
    {
      fprintf (stderr, "ERROR: couldn't read configuration file: %s\n", econf_errString(error));
      return 1;
    }

  if (!econf_getStringValue (key_file, "", NULL, &val))
    {
      fprintf (stderr, "ERROR: return values for NULL key_file are wrong!\n");
      return 1;
    }
  econf_free (key_file);

  return 0;
}
