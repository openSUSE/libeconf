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
  econf_file *key_file = (econf_file *)-1;
  econf_err error;
  char *path;

  error = econf_readDirs (&key_file,
			  TESTSDIR"tst-getconfdirs1-data/usr/etc",
			  TESTSDIR"tst-getconfdirs1-data/etc",
			  "getconfdir", "conf", "=", "#");
  if (error) {
      fprintf (stderr, "ERROR: econf_readDirs: %s\n",
	       econf_errString(error));
      return 1;
  }

  path = econf_getPath(key_file);
  if (strlen(path) > 0) {
    fprintf (stderr,
	     "ERROR: path has to be an empty string:%s\n",
	     path);
    econf_free (key_file);
    free(path);
    return 1;    
  }

  free(path);  
  econf_free (key_file);
  key_file = NULL;
  
  error = econf_readFile (&key_file,
			  TESTSDIR"tst-merge1-data/usr/etc/tst-merge1.conf",
			  "=", "#");
  if (error || key_file == NULL) {
    fprintf (stderr, "ERROR: couldn't read /usr/etc configuration file: %s\n", econf_errString(error));
    return 1;
  }

  path = econf_getPath(key_file);  
  if (strcmp(path,
	     TESTSDIR"tst-merge1-data/usr/etc/tst-merge1.conf") != 0) {
    fprintf (stderr,
	     "ERROR: wrong path has be returned:%s\n",
	     path);
    free(path);    
    econf_free (key_file);    
    return 1;    
  }  

  free(path);  
  econf_free (key_file);

  return 0;
}
