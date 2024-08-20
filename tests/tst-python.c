#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include "libeconf_ext.h"

/* Test case:
 *  Test python style:
 *  - no comment in the same line
 *  - mulitline is represented by identations
 */

static int
print_error_get (const char *key, econf_err error)
{
  fprintf (stderr, "ERROR: tried to get '%s': %s\n",
	   key, econf_errString(error));
  return 1;
}

static bool
check (econf_file *key_file, const char *key,
       const char *value, const char *comment_before_key)
{
  econf_err error;
  econf_ext_value *ext_val;
  char *val_String;

  if ((error = econf_getExtValue(key_file, NULL, key, &ext_val)))
  {
    print_error_get (key, error);
    econf_freeExtValue(ext_val);
    return false;
  }

  if ((error = econf_getStringValue(key_file, NULL, key, &val_String)))
  {
    print_error_get (key, error);
    return false;
  }  

  if ((val_String != NULL && value == NULL) ||
      (val_String == NULL && value != NULL) ||
      (val_String != NULL && value != NULL &&
       strcmp(val_String, value) != 0))
  {
    fprintf (stderr, "ERROR: %s\nExpected String:\n'%s'\nGot:\n'%s'\n",
	     key, value, val_String);
    econf_freeExtValue(ext_val);
    free(val_String);
    return false;
  }

  free(val_String);

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

  if (ext_val->comment_after_value != NULL)
  {
    fprintf (stderr, "ERROR: %s\nNo comment_after_value expected\nGot:\n'%s'\n",
	     key, ext_val->comment_after_value);
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
    const char *const value;	  
    const char *const comment_before_key;
  } tests[] = {
    { "none_comment", "1", NULL},
    { "none_comment_with_delim", "1=test", NULL},
    { "header_single", "string with spaces", " header_single_line test"},
    { "header_single_with_delim", "string with spaces = 3", " header_single_line test with delim"},
    { "header_with_value", "string with spaces #value description", " header_with_value test"},
    { "multiline_header", "multiline header", " line 1\n line 2\n\n line 3"},
    { "multiline_entry", "line one # line 1\nline two # line 2", " header multiline"},
    { "multiline_entry_with_delim", "\nline one # line 1\nline two=\nline three= # line 3", NULL},
  };
  unsigned int i;

  error = econf_newKeyFile_with_options(&key_file, "PYTHON_STYLE=1");
  if (error != ECONF_SUCCESS)
  {
    fprintf (stderr, "ERROR: couldn't create new key_file: %s\n",
             econf_errString(error));
    return 1;
  }

  error = econf_readConfig (&key_file,
			    NULL,
			    TESTSDIR"tst-python-data",
			    "arguments", "conf",
			    "=", "#");
  if (error)
  {
    fprintf (stderr, "ERROR: couldn't read configuration file: %s\n", econf_errString(error));
    char *filename;
    uint64_t line_nr;
    econf_errLocation (&filename, &line_nr);
    fprintf (stderr, "ERROR: in file: %s:%ld\n", filename, line_nr);
    free(filename);
    return 1;
  }

  for (i = 0; i < sizeof(tests)/sizeof(*tests); i++)
  {
    if (!check(key_file, tests[i].key, tests[i].value, tests[i].comment_before_key))
      retval = 1;
  }

  econf_free(key_file);
  return retval;  
}
