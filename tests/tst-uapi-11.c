#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include "libeconf.h"
bool callback_without_error(const char *filename, const void *data);
bool callback_with_error(const char *filename, const void *data);

/* Test case:

   /etc/foo/bar.conf exists
   /etc/foo/bar.conf.d/a.conf exists
   /etc/foo/bar.conf.d/b.conf exists

   libeconf should read /etc/foo/bar.conf, /etc/foo/bar.conf.d/a.conf
   and /etc/foo/bar.conf.d/b.conf in this order and should call a callback.
*/

bool callback_without_error(const char *filename, const void *data) {
  fprintf (stderr,"Callback for file: %s; data: %s.\n",
	   filename, (const char*)data);	
  return true;
}

bool callback_with_error(const char *filename, const void *data) {
  fprintf (stderr,"Callback for file: %s; data: %s.\n",
	   filename, (const char*)data);
  fprintf (stderr,"Stopping parsing\n");
  return false;
}

int
main(void)
{
  econf_file *key_file = NULL;
  int retval = 0;
  econf_err error;

  error = econf_readConfigWithCallback (&key_file,
					"foo",
					"/usr/etc",
					"bar",
					"conf", "=", "#",
					callback_without_error,
					(void *) "test without error");
  if (error)
    {
      fprintf (stderr, "ERROR: econf_readConfigWithCallback: %s\n",
	       econf_errString(error));
      return 1;
    }

  econf_free (key_file);
  key_file = NULL;

  error = econf_readConfigWithCallback (&key_file,
					"foo",
					"/usr/etc",
					"bar",
					"conf", "=", "#",
					callback_with_error,
					(void *) "test with error");
  if (error != ECONF_PARSING_CALLBACK_FAILED)
  {
    fprintf (stderr, "ERROR: expecting: %s\n       returned:  %s\n",
	     econf_errString(ECONF_PARSING_CALLBACK_FAILED),
	     econf_errString(error));
    retval = 1;
  }

  econf_free (key_file);

  return retval;
}
