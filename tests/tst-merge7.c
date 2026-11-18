#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include "libeconf.h"
#include "libeconf_ext.h"

/* When a snippet (etc) overrides a key already present in the base (usr)
 * file, the merged entry must take its line number and comment from the
 * overriding (etc) entry, not keep the metadata of the overridden (usr)
 * entry while only the value gets updated.
*/

int
main(void)
{
  econf_file *key_file_usr = NULL, *key_file_etc = NULL, *key_file_m = NULL;
  econf_err error;
  int retval = 0;

  error = econf_readFile (&key_file_usr, TESTSDIR"tst-merge7-data/data1.conf", "=", "#");
  if (error || key_file_usr == NULL)
    {
      fprintf (stderr, "ERROR: couldn't read data1.conf: %s\n", econf_errString(error));
      return 1;
    }
  error = econf_readFile (&key_file_etc, TESTSDIR"tst-merge7-data/data2.conf", "=", "#");
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

  econf_ext_value *ext_val = NULL;
  error = econf_getExtValue (key_file_m, "Server", "ListenAddress", &ext_val);
  if (error)
    {
      fprintf (stderr, "ERROR: couldn't get ListenAddress: %s\n", econf_errString(error));
      retval = 1;
      goto out;
    }

  if (strcmp (ext_val->values[0], "etc_value") != 0)
    {
      fprintf (stderr, "ERROR: expected value 'etc_value', got '%s'\n", ext_val->values[0]);
      retval = 1;
    }

  if (ext_val->line_number != 4)
    {
      fprintf (stderr, "ERROR: expected line_number 4 (from overriding etc file), got %lu\n",
	       (unsigned long) ext_val->line_number);
      retval = 1;
    }

  if (ext_val->comment_before_key == NULL ||
      strstr (ext_val->comment_before_key, "etc override comment") == NULL)
    {
      fprintf (stderr, "ERROR: expected comment from overriding etc file, got '%s'\n",
	       ext_val->comment_before_key ? ext_val->comment_before_key : "(null)");
      retval = 1;
    }

  econf_freeExtValue (ext_val);

 out:
  if (key_file_usr)
    econf_free (key_file_usr);
  if (key_file_etc)
    econf_free (key_file_etc);
  if (key_file_m)
    econf_free (key_file_m);

  return retval;
}
