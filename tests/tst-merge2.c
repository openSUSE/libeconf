#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>

#include "libeconf.h"

/* Test case:
 * Try to open two files, where one does not exist.
 * Call econf_mergeFiles with this.
 * Application should not crash.
*/

int
main(void)
{
  econf_file *key_file_1 = (econf_file *)-1, *key_file_2 = (econf_file *)-1, *key_file_m = (econf_file *)-1;
  econf_err error;

  error = econf_readFile (&key_file_1, TESTSDIR"tst-merge2-data/etc/tst-merge2.conf", "=", "#");
  if (error || key_file_1 == NULL)
    {
      fprintf (stderr, "ERROR: couldn't read /etc configuration file: %s\n", econf_errString(error));
      return 1;
    }
  error = econf_readFile (&key_file_2, TESTSDIR"tst-merge2-data/usr/etc/tst-merge2.conf", "=", "#");
  if (!error || key_file_2 != NULL)
    {
      fprintf (stderr, "ERROR: /usr/etc should not contain a config file\n");
      return 1;
    }

  error = econf_mergeFiles (&key_file_m, key_file_2, key_file_1);
  if (key_file_m != NULL)
    {
      fprintf (stderr, "ERROR: econf_mergeFiles merged somethng when there was nothing to merge\n");
      return 1;
    }


  if (key_file_1)
    econf_free (key_file_1);
  if (key_file_2)
    econf_free (key_file_2);
  if (key_file_m)
    econf_free (key_file_m);

  return 0;
}
