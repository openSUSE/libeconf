#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>

#include "libeconf.h"

/* Test case:
   Try to open a non existing configuration file and cleanup afterwards
*/

int
main(int argc, char **argv)
{
  Key_File *key_file = NULL, *key_file_1 = NULL, *key_file_2 = NULL,
    *key_file_m = NULL;

  key_file_1 = econf_get_key_file ("tst-merge1-data/etc/tst-merge1.conf", "=", '#');
  key_file_2 = econf_get_key_file ("tst-merge1-data/usr/etc/tst-merge1.conf", "=", '#');
  if (key_file_1 == NULL && key_file_2 == NULL)
    {
      fprintf (stderr, "No config file found\n");
      return 1;
    }

  if (key_file_1 != NULL && key_file_2 != NULL)
    {
      key_file_m = econf_merge_key_files (key_file_2, key_file_1);
      if (key_file_m == NULL)
	{
	  fprintf (stderr, "Cannot merge config files\n");
	  return 1;
	}
    }

  if (key_file_1)
    econf_destroy (key_file_1);
  if (key_file_2)
    econf_destroy (key_file_2);
  if (key_file_m)
    econf_destroy (key_file_m);

  return 0;
}
