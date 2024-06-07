#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>

#include "libeconf.h"

/* Test case:
   Try to open a non existing configuration file and cleanup afterwards
*/

int
main(void)
{
  econf_file *key_file = (econf_file *)-1;
  econf_err error;

  error = econf_readFile (&key_file, "doesnotexist1.conf", "=", "#");
  if (key_file)
    {
      fprintf (stderr, "Got key_file for non-existing configuration file!\n");
      econf_free(key_file);
      return 1;
    }
  if (error != ECONF_NOFILE)
    {
      fprintf (stderr, "Wrong error code: [%i] %s\n", error, econf_errString(error));
      return 1;
    }

  return 0;
}
