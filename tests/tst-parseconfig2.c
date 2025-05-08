#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include "libeconf.h"

/* Test case:
   Open file comment sign in comment line
*/

int
main(void)
{
  __attribute__ ((__cleanup__(econf_freeFilep))) econf_file *key_file = NULL;
  char *val = NULL;
  econf_err error;

  if ((error = econf_readFile (&key_file, TESTSDIR"tst-parseconfig-data/osrelease.conf", "=", "#")))
    {
      fprintf (stderr, "ERROR: couldn't read configuration file: %s\n", econf_errString(error));
      return 1;
    }


  if ((error = econf_getStringValue(key_file, NULL, "ANSI_COLOR", &val)))
    {
      fprintf(stderr, "ERROR: couldn't get key 'ANSI_COLOR': %s\n", econf_errString(error));
      return 1;
    }

  if (val[strlen(val)-1] == '\n')
    {
      fprintf(stderr, "ERROR: ANSI_COLOR has extra '\\n'!\n");
      free(val);
      return 1;
    }

  free (val);

  key_file = econf_free (key_file);

  return 0;
}
