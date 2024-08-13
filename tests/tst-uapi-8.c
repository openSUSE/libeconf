#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include "libeconf.h"

/* Test case:

   /usr/lib/foo/bar.conf exists
   /etc/foo/bar.conf exists but is empty
   /usr/lib/foo/bar.conf.d/a.conf exists
   /etc/foo/bar.conf.d/a.conf exists but is empty.

   As all relevant files are masked, not data will be read.
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
                            "/usr/lib",
			    "bar",
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
  
  econf_free (key_file);

  return retval;
}
