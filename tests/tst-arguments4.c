#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include "libeconf.h"

/* Test case:
 * Provide a NULL pointer for key_file. We should return an error, not crash
*/

int
main(int argc, char **argv)
{
  Key_File *key_file;
  const char *val;
  econf_err error;

  key_file = econf_get_key_file (TESTSDIR"tst-arguments-data/etc/arguments.conf", "=", '#', &error);
  if (key_file == NULL)
    {
      fprintf (stderr, "ERROR: couldn't read configuration file: %s\n", econf_errString(error));
      return 1;
    }

  error = ECONF_SUCCESS;
  val = econf_getStringValue (NULL, "", "KEY", &error);
  if (val != NULL && error != ECONF_ERROR)
    {
      fprintf (stderr, "ERROR: return values for NULL key_file are wrong!\n");
      return 1;
    }

  econf_destroy (key_file);

  return 0;
}
