#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>

#include "libeconf.h"

/* Test case:
 * Try to open two files, where one does not exist.
 * Call econf_merge_key_files with this.
 * Application should not crash.
*/

int
main(int argc, char **argv)
{
  Key_File *key_file_1 = NULL, *key_file_2 = NULL, *key_file_m = NULL;
  econf_err error;

  error = econf_get_key_file (&key_file_1, TESTSDIR"tst-merge1-data/etc/tst-merge1.conf", "=", '#');
  if (!error || key_file_1 != NULL)
    {
      fprintf (stderr, "ERROR: /etc should not contain a config file\n");
      return 1;
    }
  error = econf_get_key_file (&key_file_2, TESTSDIR"tst-merge1-data/usr/etc/tst-merge1.conf", "=", '#');
  if (error || key_file_2 == NULL)
    {
      fprintf (stderr, "ERROR: couldn't read /usr/etc configuration file: %s\n", econf_errString(error));
      return 1;
    }

  error = econf_merge_key_files (&key_file_m, key_file_2, key_file_1);
  if (!error || key_file_m != NULL)
    {
      /* there was nothing to merge */
      fprintf (stderr, "ERROR: econf_merge_key_files merged something when there was nothing to merge: %s\n", econf_errString(error));
      return 1;
    }

  if (key_file_1)
    econf_destroy (key_file_1);
  if (key_file_2)
    econf_destroy (key_file_2);
  if (key_file_m)
    econf_destroy (key_file_m);

  return 0;
}
