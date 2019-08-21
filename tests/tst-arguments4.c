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
  char *val;

  key_file = econf_get_key_file ("tst-arguments-data/etc/arguments.conf", "=", '#');
  if (key_file == NULL)
    return 1;

  val = econf_getStringValue (NULL, "", "KEY");
  if (val != NULL && strlen(val) > 0)
    {
      fprintf (stderr, "KEY returned something!\n");
      return 1;
    }

  econf_destroy (key_file);

  return 0;
}
