#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include "libeconf.h"

/* Test case:
   Reporting parsing errors.
*/

int
main(void)
{
  econf_file *key_file = NULL;
  econf_err error;

  error = econf_readDirs (&key_file,
			  TESTSDIR"tst-parse-error/usr/etc",
			  TESTSDIR"tst-parse-error/etc",
			  "example", "conf", "=", "#");
  if (error)
  {
    fprintf (stderr, "ERROR: econf_readDirs: %s\n",
	     econf_errString(error));
    return 1;
  }

  econf_free (key_file);  

  if ((error = econf_readFile (&key_file, TESTSDIR"tst-parseconfig-data/empty-group.conf", "=", "#")))
    {
      fprintf (stderr, "ERROR: couldn't read configuration file: %s\n", econf_errString(error));
      return 1;
    }

  econf_free (key_file);

  return 0;
}
