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

  key_file_1 = econf_get_key_file (TESTSDIR"tst-merge2-data/etc/tst-merge2.conf", "=", '#', NULL);
  if (key_file_1 == NULL)
    {
      fprintf (stderr, "ERROR: couldn't read /etc configuration file: %s\n", econf_errString(error));
      return 1;
    }
  key_file_2 = econf_get_key_file (TESTSDIR"tst-merge2-data/usr/etc/tst-merge2.conf", "=", '#', &error);
  if (key_file_2 != NULL)
    {
      fprintf (stderr, "ERROR: /usr/etc should not contain a config file\n");
      return 1;
    }

  key_file_m = econf_merge_key_files (key_file_2, key_file_1, NULL);
  if (key_file_m != NULL)
    {
      fprintf (stderr, "ERROR: econf_merge_key_files merged somethng when there was nothing to merge\n");
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
