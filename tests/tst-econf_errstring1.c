#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <limits.h>

#include "libeconf.h"

/* Test Case: print more econf error strings than we did define.
   Only way to crash is by off by one errors.
 */

int
main(void)
{
  int i;

  for (i = 0; i < 20; i++)
    printf ("%.3i: %s\n", i, econf_errString(i));

  printf ("MAX: %s\n", econf_errString(INT_MAX));

  return 0;
}
