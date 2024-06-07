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

#include "libeconf.h"
#include "keyfile.h"

extern bool allow_follow_symlinks;
// Checking file permissions, uid, group,...
extern bool file_owner_set;
extern uid_t file_owner;
extern bool file_group_set;
extern gid_t file_group;
extern bool file_permissions_set;
extern mode_t file_perms_file;
extern mode_t file_perms_dir;

extern econf_err read_file_with_callback(econf_file **key_file, const char *file_name,
					    const char *delim, const char *comment,
					    bool (*callback)(const char *filename, const void *data),
					    const void *callback_data);

/* Fill the econf_file struct with values from the given file */
extern econf_err read_file(econf_file *read_file, const char *file,
			   const char *delim, const char *comment);

extern void last_scanned_file(char **filename, uint64_t *line_nr);
