#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include "libeconf.h"

/* Test case:
   Call econf_readConfig() with a non existing config
   Should not lead to a memory leak
*/

int
main(void)
{
  econf_file *key_file = NULL;
  econf_err error;

  error = econf_readConfig (&key_file,
                            "foo",
                            "/usr/etc",
                            "bar",
                            "conf", "=", "#");
  if (error != ECONF_NOFILE)
    {
      fprintf (stderr, "ERROR: econf_readConfig: %s\n",
               econf_errString(error));
      return 1;
    }

  return 0;
}
