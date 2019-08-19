#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "libeconf.h"

/* Test case:
   Try to open a non existing configuration file and cleanup afterwards
*/

int
main(int argc, char **argv)
{
  Key_File *key_file;

  key_file = econf_get_key_file ("doesnotexist1.conf", "=", '#');
  if (key_file)
    econf_destroy(key_file);

  return 0;
}
