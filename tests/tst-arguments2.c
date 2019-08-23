#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include "libeconf.h"

/* Test case:
 * Provide a NULL pointer for the group. Means there is no group.
*/

int
main(int argc, char **argv)
{
  Key_File *key_file;
  char *val;

  key_file = econf_get_key_file (TESTSDIR"tst-arguments-data/etc/arguments.conf", "=", '#');
  if (key_file == NULL)
    return 1;

  val = econf_getStringValue (key_file, NULL, "KEY");
  if (val == NULL || strlen(val) == 0)
    {
      fprintf (stderr, "KEY returns nothing!\n");
      return 1;
    }

  econf_destroy (key_file);

  return 0;
}
