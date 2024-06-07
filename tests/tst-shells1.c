#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include "libeconf.h"
#include "libeconf_ext.h"

/* Test case:
   Open default /etc/shells and try out if we can parse entries
*/

int
main(void)
{
  econf_file *key_file = (econf_file *)-1;
  char **keys;
  size_t key_number;
  char *val;
  econf_err error;

  if ((error = econf_readFile (&key_file, TESTSDIR"tst-shells1-data/etc/shells", "", "#")))
    {
      fprintf (stderr, "ERROR: couldn't read configuration file: %s\n", econf_errString(error));
      return 1;
    }

  error = econf_getKeys(key_file, NULL, &key_number, &keys);
  if (error)
    {
      fprintf (stderr, "Error getting all keys: %s\n", econf_errString(error));
      return 1;
    }

  if (key_number == 0)
    {
      fprintf (stderr, "No keys found?\n");
      return 1;
    } else {
      fprintf (stderr, "%ld keys found\n", key_number);
    }
  for (size_t i = 0; i < key_number; i++)
    {
      printf ("%zu: --%s--\n", i, keys[i]);
    }

  if ((error = econf_getStringValue (key_file, NULL, "/usr/bin/bash", &val)))
    {
      fprintf (stderr, "Error reading /usr/bin/bash: %s\n",
	       econf_errString(error));
      return 1;
    }
  else if (val != NULL && strlen(val) > 0)
    {
      fprintf (stderr, "/usr/bin/bash returns wrong value: '%s'\n", val);
      return 1;
    }
  free (val);

  if (!(error = econf_getStringValue (key_file, NULL, "doesnotexist", &val)))
    {
      fprintf (stderr, "No error looking for \"doesnotexist\"!\n");
      return 1;
    }
  free (val);

  econf_ext_value *ext_val;

  if ((error = econf_getExtValue(key_file, NULL, "/bin/bash", &ext_val)))
    {
      fprintf (stderr, "Error ext-reading /bin/bash: %s\n",
	       econf_errString(error));
      return 1;
    }

  const char *comment_before_key = " comment line 2";
  if (ext_val->comment_before_key == NULL ||
      strcmp(ext_val->comment_before_key, comment_before_key) != 0)
  {
    fprintf (stderr, "ERROR: %s\nExpected String:\n'%s'\nGot:\n'%s'\n",
	     "/usr/bin/bash", comment_before_key, ext_val->comment_before_key);
    econf_freeExtValue(ext_val);
    return false;
  }

  econf_freeExtValue(ext_val);
  if ((error = econf_getExtValue(key_file, NULL, "/bin/csh", &ext_val)))
    {
      fprintf (stderr, "Error ext-reading /bin/csh: %s\n",
	       econf_errString(error));
      return 1;
    }

  const char *comment_after_value = " comment for /bin/csh";
  if (ext_val->comment_after_value == NULL ||
      strcmp(ext_val->comment_after_value, comment_after_value) != 0)
  {
    fprintf (stderr, "ERROR: %s\nExpected String:\n'%s'\nGot:\n'%s'\n",
	     "/usr/bin/csh", comment_after_value, ext_val->comment_after_value);
    econf_freeExtValue(ext_val);
    return false;
  }
  econf_freeExtValue(ext_val);

  // Rewrite file to disk
  econf_writeFile(key_file, TESTSDIR"tst-shells1-data/", "out.ini");

  // And reading it again
  econf_file *key_compare = (econf_file *)-1;
  error = econf_readFile(&key_compare,
			 TESTSDIR"tst-shells1-data/out.ini", "", "#");
  if (error || key_compare == NULL) {
    fprintf (stderr, "ERROR: couldn't read written configuration file: %s\n", econf_errString(error));
    return 1;
  }
  remove(TESTSDIR"tst-shells1-data/out.ini");

  econf_free (keys);
  econf_free (key_file);
  econf_free (key_compare);

  return 0;
}
