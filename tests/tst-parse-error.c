#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include "libeconf.h"

/* Test case:
   Reporting parsing errors.
*/

int
main(void)
{
  econf_file *key_file = NULL;
  econf_err error;

  error = econf_readDirs (&key_file,
			  TESTSDIR"tst-parse-error/usr/etc",
			  TESTSDIR"tst-parse-error/etc",
			  "missing_bracket", "conf", "=", "#");
  econf_free (key_file);
  if (error != ECONF_MISSING_BRACKET)
  {
    fprintf (stderr, "wrong return value for missing brackets: %s\n",
	     econf_errString(error));
    return 1;
  }

  error = econf_readFile(&key_file, TESTSDIR"tst-parse-error/missing_delim.conf", "=", "#");
  econf_free (key_file);
  if (error != ECONF_MISSING_DELIMITER)
  {
    fprintf (stderr, "wrong return value for missing delimiters: %s\n",
	     econf_errString(error));
    return 1;
  }

  error = econf_readFile(&key_file, TESTSDIR"tst-parse-error/empty_section.conf", "=", "#");
  econf_free (key_file);
  if (error != ECONF_EMPTY_SECTION_NAME)
  {
    fprintf (stderr, "wrong return value for empty section names: %s\n",
	     econf_errString(error));
    return 1;
  }

  return 0;
}
