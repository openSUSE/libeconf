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
main(int argc, char **argv)
{
  Key_File *key_file;
  char *val;
  econf_err error;

  key_file = econf_get_key_file (TESTSDIR"tst-arguments-data/etc/arguments.conf", "=", '#', &error);
  if (key_file == NULL)
    {
      fprintf (stderr, "ERROR: couldn't read configuration file: %s\n", econf_errString(error));
      return 1;
    }

  if (!econf_getStringValue (key_file, "", NULL, &val) || val != NULL)
    {
      fprintf (stderr, "ERROR: return values for NULL key_file are wrong!\n");
      return 1;
    }
  econf_destroy (key_file);

  return 0;
}
