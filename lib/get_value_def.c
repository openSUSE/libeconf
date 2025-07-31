/*
  Copyright (C) 2019 SUSE LLC
  Author: Thorsten Kukuk <kukuk@suse.com>

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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include "libeconf.h"

#define str_copy def!=NULL ? (*result=strdup(def)) : (*result=NULL)
#define value_copy *result=def

#define econf_getValueDef(FCT_TYPE, TYPE, COPY)				\
econf_err econf_get ## FCT_TYPE ## ValueDef(econf_file *ef, const char *group, \
					     const char *key, TYPE *result, TYPE def) { \
  if (!ef) \
    return ECONF_ERROR; \
\
  econf_err error = econf_get ## FCT_TYPE ## Value(ef, group, key, result);	\
  if (error == ECONF_NOKEY) \
    COPY;		    \
  return error; \
}


econf_getValueDef(Int, int32_t, value_copy)
econf_getValueDef(Int64, int64_t, value_copy)
econf_getValueDef(UInt, uint32_t, value_copy)
econf_getValueDef(UInt64, uint64_t, value_copy)
econf_getValueDef(Float, float, value_copy)
econf_getValueDef(Double, double, value_copy)
econf_getValueDef(String, char *, str_copy)
econf_getValueDef(Bool, bool, value_copy)
