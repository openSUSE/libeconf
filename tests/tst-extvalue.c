#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include "libeconf_ext.h"

/* Test case:
 *  Tests several string format. The result is a econf_ext_value
 *  struct. See tst-arguments-string/etc/arguments.conf.
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
		   const size_t value_lines, const uint64_t line_nr,
		   const char *filename)
{
  econf_err error;
  econf_ext_value *ext_val;

  if ((error = econf_getExtValue(key_file, group, key, &ext_val)))
  {
    print_error_get (group, key, error);
    return false;
  }

  if(ext_val->line_number != line_nr)
  {
    fprintf (stderr, "ERROR: %s:Expected line_nr:%d, got:%d\n",
	     key, (int) line_nr, (int)(ext_val->line_number));
    econf_freeExtValue(ext_val);
    return false;
  }

  if (strcmp(ext_val->file, filename))
  {
    fprintf (stderr, "ERROR: Expected path:%s, got:%s\n",
	     filename, ext_val->file);
    econf_freeExtValue(ext_val);
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
    const char *const val[3];
    const size_t lines;
    const uint64_t line_nr;
  } tests[] = {
    { "string_empty", {""}, 1, 4 },
    { "string_with_spaces", {"string with spaces"}, 1, 5 },
    { "string_escaped_with_leading_and_trailing_spaces", {"string with spaces"}, 1, 6 },
    { "string_with_newlines", {"line one","line two"}, 2, 8 },
    { "string_list_multiple_lines", {"line one","line two"}, 2, 11 },
    { "string_escaped_with_newlines", {"\"line one\n    line two\""}, 1, 13 },
    { "string_with_quotes", {"\\\""}, 1, 14 },
    { "string_with_quotes_v2", {"\\\""}, 1, 15 }
  };
  
  error = econf_readFile (&key_file, TESTSDIR"tst-arguments-string/etc/arguments.conf", "=", "#");
  if (error)
  {
    fprintf (stderr, "ERROR: couldn't read configuration file: %s\n", econf_errString(error));
    return 1;
  }

  for (size_t i = 0; i < sizeof(tests)/sizeof(*tests); i++)
  {
    if (!check_StringArray(key_file, "main", tests[i].key, tests[i].val, tests[i].lines,
			   tests[i].line_nr, TESTSDIR"tst-arguments-string/etc/arguments.conf"))
      retval = 1;
  }

  econf_free(key_file);
  return retval;
}
