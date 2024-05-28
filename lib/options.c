/*
  Copyright (C) 2024 SUSE LLC

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#include "libeconf.h"
#include "options.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CONFIG_DIRS "CONFIG_DIRS="

/*General options which can be set with econf_set_opt */
static bool join_same_entries = false;
static bool python_style = false;
static const char **parse_dirs = {NULL};
static int parse_dirs_count = 0;

extern void econf_set_opt(const char *option)
{
  if (strcmp(option, "JOIN_SAME_ENTRIES=1") == 0) {
    join_same_entries = true;
    return;
  }
  if (strcmp(option, "JOIN_SAME_ENTRIES=0") == 0) {
    join_same_entries = false;
    return;
  }
  if (strcmp(option, "PYTHON_STYLE=1") == 0) {
    python_style = true;
    return;
  }
  if (strcmp(option, "PYTHON_STYLE=0") == 1) {
    python_style = false;
    return;
  }

  if (strncmp(option, CONFIG_DIRS, strlen(CONFIG_DIRS)) == 0) {
    parse_dirs_count = 0;
    parse_dirs = malloc(sizeof(char *));

    char* in_Ptr = strdup(option + strlen(CONFIG_DIRS));
    char* o_Ptr;
    while ((o_Ptr = strsep(&in_Ptr, ":")) != NULL) {
      parse_dirs = realloc(parse_dirs, (++parse_dirs_count+1) * sizeof(char *));
      parse_dirs[parse_dirs_count-1] = strdup(o_Ptr);
    }
    parse_dirs[parse_dirs_count] = NULL;
  }
}

bool option_join_same_entries(void)
{
  return join_same_entries;
}
bool option_python_style(void)
{
  return python_style;
}

void option_parse_dirs(const char ***dirs, int *count)
{
  *count = parse_dirs_count;
  *dirs = parse_dirs;
}
