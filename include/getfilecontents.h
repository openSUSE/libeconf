/*
  Copyright (C) 2019 SUSE LLC
  Author: Pascal Arlt <parlt@suse.com>

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

#pragma once

/* --- getfilecontents.h --- */

#include <stdio.h>

#include "libeconf.h"
#include "keyfile.h"

/* This file contains the declaration of the functions used by
   econf_readFile in libeconf.c to write the contents of a config file into
   the econf_file struct.  */


/* Fill the econf_file struct with values from the given file handle */
extern econf_err fill_key_file(econf_file *read_file, FILE *kf, const char *delim);

/* Write the group/value entry to the given file_entry */
extern void end_of_line(struct file_entry **fe, size_t *len, size_t *lnum,
                        size_t vlen, char *buffer);

/* Check whether file_entry has enough memory allocated, if not realloc */
extern void new_kf_line(struct file_entry **fe, size_t *file_length, size_t *lnum);
