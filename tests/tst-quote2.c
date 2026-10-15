#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include "libeconf.h"

/* Test case:
   Check that the "quotes" flag of newly created (not parsed from a file)
   entries can be controlled via the DEFAULT_QUOTES option of
   econf_newKeyFile_with_options as well as via econf_setQuotes, and that
   econf_writeFile honors this flag by enclosing the written value in
   quotes.
*/

static int
file_contains(const char *path, const char *needle)
{
  FILE *fp = fopen(path, "r");
  if (fp == NULL)
    {
      fprintf (stderr, "ERROR: couldn't open '%s'\n", path);
      return 0;
    }

  char buf[4096];
  size_t len = fread(buf, 1, sizeof(buf) - 1, fp);
  buf[len] = '\0';
  fclose(fp);

  return strstr(buf, needle) != NULL;
}

int
main(void)
{
  econf_err error;
  int retval = 0;
  const char *out_file = "tst-quote2-out.ini";
  const char *out_path = TESTSDIR "tst-quote2-out.ini";

  /* --- Part 1: default behaviour (DEFAULT_QUOTES not set) --- */

  econf_file *kf = NULL;
  if ((error = econf_newKeyFile_with_options(&kf, "")))
    {
      fprintf (stderr, "ERROR: couldn't create new econf_file: %s\n",
	       econf_errString(error));
      return 1;
    }
  econf_set_delimiter_tag(kf, '=');
  econf_set_comment_tag(kf, '#');

  if ((error = econf_setStringValue(kf, "test", "plain", "value1")))
    {
      fprintf (stderr, "ERROR: couldn't set 'plain': %s\n", econf_errString(error));
      retval = 1;
    }

  if (econf_hasQuotes(kf, "test", "plain"))
    {
      fprintf (stderr, "ERROR: 'plain' should not have quotes by default\n");
      retval = 1;
    }

  if ((error = econf_setStringValue(kf, "test", "forced_quoted", "value2")))
    {
      fprintf (stderr, "ERROR: couldn't set 'forced_quoted': %s\n", econf_errString(error));
      retval = 1;
    }

  if ((error = econf_setQuotes(kf, "test", "forced_quoted", true)))
    {
      fprintf (stderr, "ERROR: couldn't set quotes for 'forced_quoted': %s\n",
	       econf_errString(error));
      retval = 1;
    }

  if (!econf_hasQuotes(kf, "test", "forced_quoted"))
    {
      fprintf (stderr, "ERROR: 'forced_quoted' should have quotes after econf_setQuotes\n");
      retval = 1;
    }

  if ((error = econf_writeFile(kf, TESTSDIR, out_file)))
    {
      fprintf (stderr, "ERROR: couldn't write '%s': %s\n", out_path, econf_errString(error));
      retval = 1;
    }
  else
    {
      if (!file_contains(out_path, "plain=value1"))
	{
	  fprintf (stderr, "ERROR: expected unquoted 'plain=value1' in written file\n");
	  retval = 1;
	}
      if (!file_contains(out_path, "forced_quoted=\"value2\""))
	{
	  fprintf (stderr, "ERROR: expected quoted 'forced_quoted=\"value2\"' in written file\n");
	  retval = 1;
	}
      remove(out_path);
    }

  econf_free(kf);

  /* --- Part 2: DEFAULT_QUOTES=1 --- */

  econf_file *kf_def = NULL;
  if ((error = econf_newKeyFile_with_options(&kf_def, "DEFAULT_QUOTES=1")))
    {
      fprintf (stderr, "ERROR: couldn't create new econf_file with DEFAULT_QUOTES=1: %s\n",
	       econf_errString(error));
      return 1;
    }
  econf_set_delimiter_tag(kf_def, '=');
  econf_set_comment_tag(kf_def, '#');

  if ((error = econf_setStringValue(kf_def, "test", "default_quoted", "value3")))
    {
      fprintf (stderr, "ERROR: couldn't set 'default_quoted': %s\n", econf_errString(error));
      retval = 1;
    }

  if (!econf_hasQuotes(kf_def, "test", "default_quoted"))
    {
      fprintf (stderr, "ERROR: 'default_quoted' should have quotes due to DEFAULT_QUOTES=1\n");
      retval = 1;
    }

  if ((error = econf_setStringValue(kf_def, "test", "forced_unquoted", "value4")))
    {
      fprintf (stderr, "ERROR: couldn't set 'forced_unquoted': %s\n", econf_errString(error));
      retval = 1;
    }

  if ((error = econf_setQuotes(kf_def, "test", "forced_unquoted", false)))
    {
      fprintf (stderr, "ERROR: couldn't clear quotes for 'forced_unquoted': %s\n",
	       econf_errString(error));
      retval = 1;
    }

  if (econf_hasQuotes(kf_def, "test", "forced_unquoted"))
    {
      fprintf (stderr, "ERROR: 'forced_unquoted' should not have quotes after econf_setQuotes\n");
      retval = 1;
    }

  if ((error = econf_writeFile(kf_def, TESTSDIR, out_file)))
    {
      fprintf (stderr, "ERROR: couldn't write '%s': %s\n", out_path, econf_errString(error));
      retval = 1;
    }
  else
    {
      if (!file_contains(out_path, "default_quoted=\"value3\""))
	{
	  fprintf (stderr, "ERROR: expected quoted 'default_quoted=\"value3\"' in written file\n");
	  retval = 1;
	}
      if (!file_contains(out_path, "forced_unquoted=value4"))
	{
	  fprintf (stderr, "ERROR: expected unquoted 'forced_unquoted=value4' in written file\n");
	  retval = 1;
	}
      remove(out_path);
    }

  econf_free(kf_def);

  return retval;
}
