/*
  Copyright (C) 2019 SUSE LLC
  Author: Pascal Arlt <parlt@suse.com>
  Author: Dominik Gedon <dgedon@suse.com>

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

/**
 * @file getfilecontents.h
 * @brief Fills the econf_file struct with data from a given file.
 */

#pragma once

#include "libeconf.h"
#include "keyfile.h"

/**
 * @brief Read the file line by line and parse for comments, keys and values.
 * @param read_file The econf_file struct where the data will be stored.
 * @param file The file to read from.
 * @param delim A char used to assign a value to a key.
 * @param comment Specifies which char to regard as comment indicator.
 * @return An econf error code. See libeconf.h for details.
 */
extern econf_err read_file(econf_file *read_file, const char *file,
			   const char *delim, const char *comment);
