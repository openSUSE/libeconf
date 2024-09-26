#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include "libeconf.h"

/* Test case:
 *  reading lines which are longer than BUFSIZ
 */


int
main(void)
{
  econf_file *key_file = (econf_file *)-1;
  econf_err error;
  int retval = 0;
  char **keys = NULL;
  size_t key_number = 0;

  error = econf_readFile (&key_file, TESTSDIR"tst-long-name/test.conf", "=", "#");
  if (error)
  {
    fprintf (stderr, "ERROR: couldn't read configuration file: %s\n", econf_errString(error));
    return 1;
  }

  error = econf_getKeys(key_file, NULL, &key_number, &keys);
  if (error != ECONF_SUCCESS) {
    econf_free(key_file);	  
    fprintf(stderr, "Unable to read keys: %s",
	    econf_errString(error));
    return 3;
  }
  
  for (size_t i = 0; i < key_number; i++) {
    char *val;
    error = econf_getStringValue(key_file, NULL, keys[i], &val);
    if (error != ECONF_SUCCESS) {
      fprintf(stderr, "Unable to get string from key %s: %s",
	      keys[i], econf_errString(error));
      return 4;
    }
    if (strlen(keys[i])!=8200) {
      fprintf(stderr, "wrong length of key %s\n",
	      keys[i]);
      return 5;
    }
    if (strcmp(val,"b") != 0) {
      fprintf(stderr, "wrong value %s\n",
	      val);
      return 6;
    }	    
    printf("key: '%s', value: '%s'\n", keys[i], val);
    free(val);
  }
  econf_free(keys);
  econf_free(key_file);
  return retval;  
}
