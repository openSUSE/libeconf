#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include "libeconf.h"

/* Test case:
 * Merge a "usr" file whose only group has every key commented out (so it
 * parses with zero key/value entries, but the group name is still known)
 * with an "etc" file which adds a real key to that same group.
 *
 * This reproduces a crash seen in sd-report-collector: a shipped
 * /usr/lib/<project>/<project>.conf with all keys commented out, merged
 * with a /etc/<project>/<project>.conf.d/\*.conf drop-in adding real keys,
 * segfaulted in merge_existing_groups() because of a size_t underflow
 * (i - 1 with i == 0) when indexing into the usr file's (empty) entries.
 *
 * Application should not crash and should return the value from the etc
 * (drop-in) file.
*/

static bool
check_string (econf_file *key_file, const char *group, const char *key,
	      const char *value)
{
  econf_err error;

  char *val_String;
  if ((error = econf_getStringValue(key_file, group, key, &val_String)))
    {
      fprintf (stderr, "ERROR: reading (%s,%s): %s\n", group, key, econf_errString(error));
      return false;
    }
  printf ("reading (%s,%s): got '%s', expected '%s'\n", group, key, val_String, value?value:"");
  if (strcmp(val_String, value?value:"") != 0)
    {
      fprintf (stderr, "ERROR: reading (%s,%s): expected '%s', got: '%s'\n",
	       group, key, value, val_String);
      free(val_String);
      return false;
    }
  free(val_String);
  return true;
}

int
main(void)
{
  econf_file *key_file_usr = NULL, *key_file_etc = NULL, *key_file_m = NULL;
  econf_err error;
  int retval = 0;

  error = econf_readFile (&key_file_usr, TESTSDIR"tst-merge6-data/data1.conf", "=", "#");
  if (error || key_file_usr == NULL)
    {
      fprintf (stderr, "ERROR: couldn't read data1.conf: %s\n", econf_errString(error));
      return 1;
    }
  error = econf_readFile (&key_file_etc, TESTSDIR"tst-merge6-data/data2.conf", "=", "#");
  if (error || key_file_etc == NULL)
    {
      fprintf (stderr, "ERROR: couldn't read data2.conf: %s\n", econf_errString(error));
      return 1;
    }

  error = econf_mergeFiles (&key_file_m, key_file_usr, key_file_etc);
  if (error || key_file_m == NULL)
    {
      fprintf (stderr, "ERROR: error merging configuration files: %s\n", econf_errString(error));
      return 1;
    }

  if (!check_string (key_file_m, "Server", "ListenAddress", "127.0.0.1:9443")) retval = 1;

  if (key_file_usr)
    econf_free (key_file_usr);
  if (key_file_etc)
    econf_free (key_file_etc);
  if (key_file_m)
    econf_free (key_file_m);

  return retval;
}
