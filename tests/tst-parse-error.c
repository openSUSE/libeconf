#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include "libeconf.h"

/* Test case:
   Reporting parsing errors.
*/

int
main(void)
{
  econf_file *key_file = (econf_file *)-1;
  econf_err error;
  char *filename = NULL;
  uint64_t line_nr = 0;

  error = econf_readDirs (&key_file,
			  TESTSDIR"tst-parse-error/usr/etc",
			  TESTSDIR"tst-parse-error/etc",
			  "missing_bracket", "conf", "=", "#");
  econf_free (key_file);
  key_file = NULL;

  if (error != ECONF_MISSING_BRACKET)
  {
    fprintf (stderr, "wrong return value for missing brackets: %s\n",
	     econf_errString(error));
    return 1;
  }
  econf_errLocation( &filename, &line_nr);
  if (strcmp(filename,TESTSDIR"tst-parse-error/etc/missing_bracket.conf.d/missing_bracket.conf")!=0 || line_nr != 4)
  {
    fprintf (stderr, "wrong error info for parsing a text with missing bracket: %s: %d\n", filename, (int) line_nr);
    free(filename);
    return 1;
  }
  free(filename);

  error = econf_readFile(&key_file, TESTSDIR"tst-parse-error/missing_delim.conf", "=", "#");
  econf_free (key_file);
  if (error != ECONF_MISSING_DELIMITER)
  {
    fprintf (stderr, "wrong return value for missing delimiters: %s\n",
	     econf_errString(error));
    return 1;
  }
  econf_errLocation( &filename, &line_nr);
  if (strcmp(filename,TESTSDIR"tst-parse-error/missing_delim.conf")!=0 || line_nr != 3)
  {
    fprintf (stderr, "wrong error info for parsing a text with missing delimiters: %s: %d\n", filename, (int) line_nr);
    free(filename);
    return 1;
  }
  free(filename);

  error = econf_readFile(&key_file, TESTSDIR"tst-parse-error/empty_section.conf", "=", "#");
  econf_free (key_file);
  if (error != ECONF_EMPTY_SECTION_NAME)
  {
    fprintf (stderr, "wrong return value for empty section names: %s\n",
	     econf_errString(error));
    return 1;
  }
  econf_errLocation( &filename, &line_nr);
  if (strcmp(filename,TESTSDIR"tst-parse-error/empty_section.conf")!=0 || line_nr != 4)
  {
    fprintf (stderr, "wrong error info for parsing a text after an empty section: %s: %d\n", filename, (int) line_nr);
    free(filename);
    return 1;
  }
  free(filename);

  error = econf_readFile(&key_file, TESTSDIR"tst-parse-error/text_after_section.conf", "=", "#");
  econf_free (key_file);
  if (error != ECONF_TEXT_AFTER_SECTION)
  {
    fprintf (stderr, "wrong return value for parsing a text after a section: %s\n",
	     econf_errString(error));
    return 1;
  }
  econf_errLocation( &filename, &line_nr);
  if (strcmp(filename,TESTSDIR"tst-parse-error/text_after_section.conf")!=0 || line_nr != 4)
  {
    fprintf (stderr, "wrong error info for parsing a text after a section: %s: %d\n", filename, (int) line_nr);
    free(filename);
    return 1;
  }
  free(filename);

  return 0;
}
