#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include "libeconf.h"
#include "../lib/helpers.h"
#include "../lib/mergefiles.h"
#include "../lib/readconfig.h"

/* Test case:
   Test the systemd like behavior:
   /usr/lib/foo/etc/getconfdir.conf exists
   /run/foo/getconfdir.conf exists
   /etc/foo/getconfidr.conf is linked to /dev/null

   No data will be available.
*/

int
main(void)
{
  econf_file *key_file = NULL;
  int retval = 0;
  econf_err error;
  char **keys;
  size_t key_number;
  
  error = econf_readConfig (&key_file,
	                    "foo",
                            "/usr/etc",
			    "getconfdir",
			    "conf", "=", "#");  
  if (error)
    {
      fprintf (stderr, "ERROR: econf_readConfig: %s\n",
	       econf_errString(error));
      return 1;
    }

  error = econf_getKeys(key_file, NULL, &key_number, &keys);
  if (error && error != ECONF_NOKEY)
    {
      fprintf (stderr, "Error getting all keys: %s\n", econf_errString(error));
      retval = 1;
    }

  if (key_number > 0)
    {
      fprintf (stderr, "There should be no key. Key numbers: %ld\n", key_number);
      retval = 1;      
    }
  
  econf_free (keys);
  econf_free (key_file);

  return retval;
}
