#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "libeconf.h"

/* Test case:
 *  Test userID, groupID, permissions of a parsed file.
 */

int
main(void)
{
  econf_file *key_file = (econf_file *)-1;
  econf_err error;

  /* no security restrictions are set */
  error = econf_readFile (&key_file, TESTSDIR"tst-arguments-string/etc/arguments.conf", "=", "#");
  if (error)
  {
    fprintf (stderr, "ERROR: couldn't read configuration file: %s\n", econf_errString(error));
    return 1;
  }
  econf_free(key_file);

  /* File has correct user */
  econf_requireOwner(9000);
  error = econf_readFile (&key_file, TESTSDIR"tst-arguments-string/etc/arguments.conf", "=", "#");
  if (error != ECONF_WRONG_OWNER)
  {
    fprintf (stderr, "ERROR: expecting: %s\n       returned:  %s\n",
	     econf_errString(ECONF_WRONG_OWNER),
	     econf_errString(error));
    return 1;
  }
  econf_reset_security_settings();

  /* File has correct group */
  econf_requireGroup(49);
  error = econf_readFile (&key_file, TESTSDIR"tst-arguments-string/etc/arguments.conf", "=", "#");
  if (error != ECONF_WRONG_GROUP)
  {
    fprintf (stderr, "ERROR: expecting: %s\n       returned:  %s\n",
	     econf_errString(ECONF_WRONG_GROUP),
	     econf_errString(error));
    return 1;
  }
  econf_reset_security_settings();

  /* checking link */
  if (symlink(TESTSDIR"tst-arguments-string/etc/arguments.conf", TESTSDIR"tst-arguments-string/etc/link.conf") == -1)
  {
    fprintf (stderr, "WARNING: Cannot create sym link %s for testing.\n", TESTSDIR"tst-arguments-string/etc/link.conf");
    fprintf (stderr, "Exit without reporting an error");
    return 0;
  }
  econf_followSymlinks(false);
  error = econf_readFile (&key_file, TESTSDIR"tst-arguments-string/etc/link.conf", "=", "#");
  remove(TESTSDIR"tst-arguments-string/etc/link.conf");
  if (error != ECONF_ERROR_FILE_IS_SYM_LINK)
  {
    fprintf (stderr, "ERROR: expecting: %s\n       returned:  %s\n",
	     econf_errString(ECONF_ERROR_FILE_IS_SYM_LINK),
	     econf_errString(error));
    return 1;
  }
  econf_reset_security_settings();

  /* checking file permissions */
  mode_t other_execute_right = S_IXOTH;
  mode_t other_read_right = S_IROTH;
  mode_t other_write_right = S_IWOTH;
  econf_requirePermissions(other_execute_right, other_read_right);
  error = econf_readFile (&key_file, TESTSDIR"tst-arguments-string/etc/arguments.conf", "=", "#");
  if (error != ECONF_WRONG_FILE_PERMISSION)
  {
    fprintf (stderr, "ERROR: expecting: %s\n       returned:  %s\n",
	     econf_errString(ECONF_WRONG_FILE_PERMISSION),
	     econf_errString(error));
    return 1;
  }
  econf_reset_security_settings();

  /* checking directory permissions */
  econf_requirePermissions(other_read_right, other_write_right);
  error = econf_readFile (&key_file, TESTSDIR"tst-arguments-string/etc/arguments.conf", "=", "#");
  if (error != ECONF_WRONG_DIR_PERMISSION)
  {
    fprintf (stderr, "ERROR: expecting: %s\n       returned:  %s\n",
	     econf_errString(ECONF_WRONG_DIR_PERMISSION),
	     econf_errString(error));
    return 1;
  }
  econf_reset_security_settings();

  return 0;
}
