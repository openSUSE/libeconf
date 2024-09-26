#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include "libeconf.h"
#include "../lib/keyfile.h"

/* Test case:
   Testing different options.
*/

int
main(void)
{
  econf_file *key_file = (econf_file *)-1;
  int retval = 0;
  econf_err error;

  if ((error = econf_newKeyFile_with_options(&key_file,
					     "PARSING_DIRS=/test/usr/etc/:" \
					     "/test/run:test/etc" \
					     ";JOIN_SAME_ENTRIES=1;PYTHON_STYLE=1")))
  {
    fprintf (stderr, "ERROR: couldn't create new econf_file: %s\n",
             econf_errString(error));
    return 1;
  }


  if (!key_file->join_same_entries) {
    fprintf (stderr, "ERROR: join_same_entries is false");
    retval = 1;
  }
  
  if (!key_file->python_style) {
    fprintf (stderr, "ERROR: python_style is false");
    retval = 1;
  }

  if (key_file->parse_dirs_count != 3) {
    fprintf (stderr, "ERROR: parse_dirs_count is not 3");
    retval = 1;
  }  

  econf_free (key_file);
  key_file = NULL;

  if (econf_newKeyFile_with_options(&key_file,
				    "PARSING_DIRS=/test/usr/etc/:"	\
				    "/test/run:test/etc"		\
				    ";UNKNOWN_TAG=1") != ECONF_OPTION_NOT_FOUND)
  {
    fprintf (stderr, "ERROR: return value has to be ECONF_OPTION_NOT_FOUND: %s\n",
	     econf_errString(error));
    return 1;
  }  

  econf_free (key_file);

  return retval;
}
