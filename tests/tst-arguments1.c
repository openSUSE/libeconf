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
main(int argc, char **argv)
{
  Key_File *key_file;
  char *val;

  key_file = econf_get_key_file ("tst-arguments/etc/arguments.conf", " =", '#');
  if (key_file == NULL)
    return 1;

  val = econf_getStringValue (key_file, "", "KEY");
  if (val == NULL || strlen(val) == 0)
    {
      fprintf (stderr, "KEY returns nothing!\n");
      return 1;
    }

  econf_destroy (key_file);

  return 0;
}
