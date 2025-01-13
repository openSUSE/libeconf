#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include "libeconf.h"
bool callback_without_error(const char *filename, const void *data);
bool callback_with_error(const char *filename, const void *data);

/* Test case:
   /etc/environment which is empty
   /usr/etc/environment.d/a.conf which is empty
   /usr/etc/environment.d/b.conf which is empty
   /usr/etc/environment.d/c.conf which is empty

   libeconf should have none entry.
*/

int
main(void)
{
  econf_file *key_file = NULL;
  int retval = 0;
  econf_err error;
  char **keys;
  size_t key_number;

  if ((error = econf_newKeyFile_with_options(&key_file, "ROOT_PREFIX="TESTSDIR)))
    {
      fprintf (stderr, "ERROR: couldn't allocate new file: %s\n",
	       econf_errString(error));
      return 1;
    }
  
  error = econf_readConfig (&key_file,
	                    NULL,
                            "/usr/etc",
			    "environment",
			    "", "=", "#");

  if (error)
    {
      fprintf (stderr, "ERROR: econf_readConfig: %s\n",
	       econf_errString(error));
      return 1;
    }


  error = econf_getKeys(key_file, NULL, &key_number, &keys);
  if (error != ECONF_NOKEY)
    {
      fprintf (stderr, "Getting all keys should return error ECONF_NOKEY: %s\n", econf_errString(error));
      return 1;
    }
  if (key_number != 0)
    {
      fprintf (stderr, "Key Number should be 0: %ld\n", key_number);
      return 1;
    }

  econf_free (key_file);

  return retval;
}
