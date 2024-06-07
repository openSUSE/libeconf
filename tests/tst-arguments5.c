#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include "libeconf.h"

/* Test case:
 * Provide NULL pointers as argument. We should return an error, not crash
*/

int
main(void)
{
  econf_file *key_file = (econf_file *)-1;
  econf_err error;

  if ((error = econf_readFile (&key_file, TESTSDIR"tst-arguments5-data/etc/arguments5.conf", NULL, NULL)))
    {
      printf ("Not crashed, but error: %s\n", econf_errString(error));
      return 0;
    }

  econf_free (key_file);

  return 0;
}
