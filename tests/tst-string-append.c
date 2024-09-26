#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include "libeconf_ext.h"

/* Test case:
 *  An string array has been defined mutitple times in a file with the
 *  same ID. If the environment JOIN_SAME_ENTRIES is set, these
 *  multiple entries are packed into an single one.
 */

static int
print_error_get (const char *getgroup, const char *key, econf_err error)
{
  fprintf (stderr, "ERROR: tried to get '%s' from '%s': %s\n",
	   key, getgroup, econf_errString(error));
  return 1;
}

static bool
check_StringArray (econf_file *key_file, const char *group,
		   const char *key, const char *const *values,
		   const size_t value_lines,
		   const char *comment_before_key,
		   const char *comment_after_value)
{
  econf_err error;
  econf_ext_value *ext_val;

  if ((error = econf_getExtValue(key_file, group, key, &ext_val)))
  {
    print_error_get (group, key, error);
    return false;
  }

  size_t i=0;
  while (ext_val->values[i] != 0)
  {
    if ((ext_val->values[i] == NULL && values[i] != NULL) ||
	(ext_val->values[i] != NULL && values[i] == NULL) ||
      strcmp(ext_val->values[i], values[i]))
    {
      fprintf (stderr, "ERROR: %s:Expected String:\n'%s'\n, Got:\n'%s'\n",
	       key, values[i], ext_val->values[i]);
      econf_freeExtValue(ext_val);
      return false;
    }
    i++;
  }

  if (i != value_lines)
  {
    fprintf (stderr,
	     "ERROR: String array does not have expected length: %d exp.: %d\n",
	     (int) i, (int) value_lines);
    econf_freeExtValue(ext_val);
    return false;    
  }

  if ((ext_val->comment_before_key != NULL && comment_before_key == NULL) ||
      (ext_val->comment_before_key == NULL && comment_before_key != NULL) ||
      (ext_val->comment_before_key != NULL && comment_before_key != NULL &&
       strcmp(ext_val->comment_before_key, comment_before_key) != 0))
  {
    fprintf (stderr, "comment_before_key ERROR: %s\nExpected String:\n'%s'\nGot:\n'%s'\n",
	     key, comment_before_key, ext_val->comment_before_key);
    econf_freeExtValue(ext_val);
    return false;
  }

  if ((ext_val->comment_after_value != NULL && comment_after_value == NULL) ||
      (ext_val->comment_after_value == NULL && comment_after_value != NULL) ||
      (ext_val->comment_after_value != NULL && comment_after_value != NULL &&
       strcmp(ext_val->comment_after_value, comment_after_value) != 0))
  {
    fprintf (stderr, "comment_after_value ERROR: %s\nExpected String:\n'%s'\nGot:\n'%s'\n",
	     key, comment_after_value, ext_val->comment_after_value);
    econf_freeExtValue(ext_val);
    return false;
  }  
  
  econf_freeExtValue(ext_val);
  return true;
}


int
main(void)
{
  econf_file *key_file = NULL;
  econf_err error;
  int retval = 0;

  static const struct {
    const char *const key;
    const char *const val[5];
    const size_t lines;
    const char *const comment_before_key;
    const char *const comment_after_value;    
  } tests[] = {
    { "append_option", {"one", "two", "three", "four"}, 4,
      " header of first entry\n header of second entry",
      "\n one\n two\nthree\nfour"},
    { "reset_option", {""}, 1,
      " header of init reset option\n header of reset option",
      NULL},
    { "reinit_option", {"three", "four"}, 2,
      " header of reinit option\n header of last reinit option",
      "three\nfour"}
  };

  /* double entries will be joined together */
  if ((error = econf_newKeyFile_with_options(&key_file, "JOIN_SAME_ENTRIES=1")))
  {
    fprintf (stderr, "ERROR: couldn't create new file: %s\n",
             econf_errString(error));
    return 1;
  }

  error = econf_readConfig (&key_file,
			    NULL,
			    TESTSDIR"tst-append-string",
			    "input", "conf",
			    "=", "#");
  if (error)
  {
    fprintf (stderr, "ERROR: couldn't read configuration file: %s\n", econf_errString(error));
    retval = 1;
  }

  for (size_t i = 0; i < sizeof(tests)/sizeof(*tests); i++)
  {
    if (!check_StringArray(key_file, "main", tests[i].key, tests[i].val, tests[i].lines,
			   tests[i].comment_before_key ,tests[i].comment_after_value))
      retval = 1;
  }

  econf_free(key_file);
  return retval;  
}
