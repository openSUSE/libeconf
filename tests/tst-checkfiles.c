#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "libeconf.h"
bool checkLink(const char *filename, const void *data);

/* Test case:
 *  Using user defined callback for checking files which will be parsed.
 */

bool checkLink(const char *filename, const void *data) {
  struct stat sb;

  fprintf (stderr,"Given string: -%s-\n", (const char*)data);

  if (lstat(filename, &sb) == -1 || (sb.st_mode&S_IFMT) == S_IFLNK)
    return false;

  return true;
 }

int
main(void)
{
  econf_file *key_file = (econf_file *)-1;
  econf_err error;

  /* checking not allowed links*/
  if (symlink(TESTSDIR"tst-arguments-string/etc/arguments.conf", TESTSDIR"tst-arguments-string/etc/link.conf") == -1)
  {
    fprintf (stderr, "ERROR: cannot create sym link: %s\n", TESTSDIR"tst-arguments-string/etc/link.conf");
    return 1;
  }
  error = econf_readFileWithCallback (&key_file, TESTSDIR"tst-arguments-string/etc/link.conf", "=", "#",
				      checkLink, (void *) "test");
  remove(TESTSDIR"tst-arguments-string/etc/link.conf");
  if (error != ECONF_PARSING_CALLBACK_FAILED)
  {
    fprintf (stderr, "ERROR: expecting: %s\n       returned:  %s\n",
	     econf_errString(ECONF_PARSING_CALLBACK_FAILED),
	     econf_errString(error));
    return 1;
  }

  return 0;
}
