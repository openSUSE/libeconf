#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include "libeconf.h"

/* Test case:
   Return absolute path if only one file has been parsed.
   Return empty string if more than one file has been parsed and merged.
*/

int
main(void)
{
  econf_file *key_file = NULL;
  econf_err error;

  error = econf_readDirs (&key_file,
			  TESTSDIR"tst-getconfdirs1-data/usr/etc",
			  TESTSDIR"tst-getconfdirs1-data/etc",
			  "getconfdir", "conf", "=", "#");
  if (error) {
      fprintf (stderr, "ERROR: econf_readDirs: %s\n",
	       econf_errString(error));
      return 1;
  }
  if (strlen(econf_getPath(key_file)) > 0) {
    fprintf (stderr,
	     "ERROR: path has to be an empty string:%s\n",
	     econf_getPath(key_file));
    econf_free (key_file);    
    return 1;    
  }

  econf_free (key_file);
  
  error = econf_readFile (&key_file,
			  TESTSDIR"tst-merge1-data/usr/etc/tst-merge1.conf",
			  "=", "#");
  if (error || key_file == NULL) {
    fprintf (stderr, "ERROR: couldn't read /usr/etc configuration file: %s\n", econf_errString(error));
    return 1;
  }

  if (strcmp(econf_getPath(key_file),
	     TESTSDIR"tst-merge1-data/usr/etc/tst-merge1.conf") != 0) {
    fprintf (stderr,
	     "ERROR: wrong path has be returned:%s\n",
	     econf_getPath(key_file));
    econf_free (key_file);    
    return 1;    
  }  

  econf_free (key_file);

  return 0;
}
