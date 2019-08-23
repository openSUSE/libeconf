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

  key_file_1 = econf_get_key_file (TESTSDIR"tst-merge2-data/etc/tst-merge2.conf", "=", '#');
  key_file_2 = econf_get_key_file (TESTSDIR"tst-merge2-data/usr/etc/tst-merge2.conf", "=", '#');
  if (key_file_1 == NULL && key_file_2 == NULL)
    {
      fprintf (stderr, "No config file found\n");
      return 1;
    }

  key_file_m = econf_merge_key_files (key_file_2, key_file_1);
  if (key_file_m != NULL) /* XXX fix when we know correct error handling */
    {
      /* there was nothing to merge */
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
