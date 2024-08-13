#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include "libeconf_ext.h"

/* Test case:
 *  Tests writing several comment. 
 */

static int
print_error_get (const char *key, econf_err error)
{
  fprintf (stderr, "ERROR: tried to get '%s': %s\n",
	   key, econf_errString(error));
  return 1;
}

static bool
check_comments (econf_file *key_file, const char *key,
		const char *comment_before_key,
		const char *comment_after_value)
{
  econf_err error;
  econf_ext_value *ext_val;

  if ((error = econf_getExtValue(key_file, NULL, key, &ext_val)))
  {
    print_error_get (key, error);
    econf_freeExtValue(ext_val);
    return false;
  }

  if ((ext_val->comment_before_key != NULL && comment_before_key == NULL) ||
      (ext_val->comment_before_key == NULL && comment_before_key != NULL) ||
      (ext_val->comment_before_key != NULL && comment_before_key != NULL &&
       strcmp(ext_val->comment_before_key, comment_before_key) != 0))
  {
    fprintf (stderr, "ERROR: %s\nExpected String:\n'%s'\nGot:\n'%s'\n",
	     key, comment_before_key, ext_val->comment_before_key);
    econf_freeExtValue(ext_val);
    return false;
  }

  if ((ext_val->comment_after_value != NULL && comment_after_value == NULL) ||
      (ext_val->comment_after_value == NULL && comment_after_value != NULL) ||
      (ext_val->comment_after_value != NULL && comment_after_value != NULL &&
       strcmp(ext_val->comment_after_value, comment_after_value) != 0))
  {
    fprintf (stderr, "ERROR: %s\nExpected String:\n'%s'\nGot:\n'%s'\n",
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
  econf_file *key_file = (econf_file *)-1;
  econf_err error;
  int retval = 0;

  static const struct {
    const char *const key;
    const char *const comment_before_key;
    const char *const comment_after_value;
  } tests[] = {
    { "none_comment", NULL, NULL },
    { "header_without_value", " header_without_value test", NULL },
    { "header_with_value", " header_with_value test", "value description" },
    { "multiline_header", " line 1\n line 2\n\n line 3", NULL },
  };
  unsigned int i;  

  error = econf_readFile (&key_file, TESTSDIR"tst-write-comments/arguments.conf", "=", "#");
  if (error)
  {
    fprintf (stderr, "ERROR: couldn't read configuration file: %s\n", econf_errString(error));
    return 1;
  }

  // Rewrite file to disk
  econf_writeFile(key_file, TESTSDIR"tst-write-comments/", "out.ini");

  // And reading it again
  econf_file *key_compare = (econf_file *)-1;
  error = econf_readFile(&key_compare,
			 TESTSDIR"tst-write-comments/out.ini", "=", "#");
  if (error || key_compare == NULL) {
    fprintf (stderr, "ERROR: couldn't read written /usr/etc configuration file: %s\n", econf_errString(error));
    return 1;
  }
  remove(TESTSDIR"tst-write-comments/out.ini");

  for (i = 0; i < sizeof(tests)/sizeof(*tests); i++)
  {
    if (!check_comments(key_compare, tests[i].key, tests[i].comment_before_key,
			tests[i].comment_after_value))
      retval = 1;
  }

  econf_free(key_file);
  econf_free(key_compare);
  return retval;  
}
