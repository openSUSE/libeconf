#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include "libeconf.h"

/* Test case:
   Open file with only empty group
*/

int
main(void)
{
  econf_file *key_file = (econf_file *)-1;
  econf_err error;

  if ((error = econf_readFile (&key_file, TESTSDIR"tst-parseconfig-data/empty-group.conf", "=", "#")))
    {
      fprintf (stderr, "ERROR: couldn't read configuration file: %s\n", econf_errString(error));
      return 1;
    }

  econf_free (key_file);

  return 0;
}
