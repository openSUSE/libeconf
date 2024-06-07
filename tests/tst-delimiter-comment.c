#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "libeconf.h"

/* Test case:
 *  get/set delimiter and comment tag of a given econf_file object
 */

int
main(void)
{
  econf_file *key_file = (econf_file *)-1;
  econf_err error;

  /* no security restrictions are set */
  error = econf_newIniFile (&key_file);
  if (error)
  {
    fprintf (stderr, "ERROR: couldn't create new ini file: %s\n", econf_errString(error));
    return 1;
  }

  if (econf_comment_tag(key_file) != '#') {
    fprintf (stderr, "ERROR:Wrong comment tag %c;  required: #\n",
	     econf_comment_tag(key_file));
    return 1;    	  
  }
  if (econf_delimiter_tag(key_file) != '=') {
    fprintf (stderr, "ERROR:Wrong delimiter tag %c;  required: =\n",
	     econf_delimiter_tag(key_file));
    return 1;    	  
  }

  econf_set_comment_tag(key_file, '-');
  econf_set_delimiter_tag(key_file, ' ');
  
  if (econf_comment_tag(key_file) != '-') {
    fprintf (stderr, "ERROR:Wrong comment tag %c;  required: -\n",
	     econf_comment_tag(key_file));
    return 1;    	  
  }
  if (econf_delimiter_tag(key_file) != ' ') {
    fprintf (stderr, "ERROR:Wrong delimiter tag %c;  required: <space>\n",
	     econf_delimiter_tag(key_file));
    return 1;    	  
  }
  
  econf_free(key_file);

  return 0;
}
