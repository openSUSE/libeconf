/*
  Copyright (C) 2021 SUSE LLC

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
/** @file libeconf_ext.h
 * @brief Public extended API for the econf library.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "libeconf.h"

#ifdef __cplusplus
extern "C" {
#endif

struct econf_ext_value {
  /** Values of a given key in form of a string array.*/
  char **values;
  /** Path of the configuration file where this value has been read. */
  char *file;
  /** Line number of the configuration key/value. */
  uint64_t line_number;
  /** Comment before the key/value entry. */
  char *comment_before_key;
  /** Comment after the value entry. */
  char *comment_after_value;
};
  
typedef struct econf_ext_value econf_ext_value;


/** @brief Evaluating more information for given group/key.
 *
 * @param kf given/parsed data
 * @param group Desired group or NULL if there is no group defined.
 * @param key Key for which the value is requested.
 * @param result A newly allocated struct or NULL in error case.
 * @return econf_err ECONF_SUCCESS or error code
 *
 */
extern econf_err econf_getExtValue(econf_file *kf, const char *group,
				   const char *key, econf_ext_value **result);

/** @brief Free an complete econf_ext_value struct.
 *
 * @param to_free struct which has to be freed
 * @return void
 *
 */
extern void econf_freeExtValue(econf_ext_value *to_free);

#ifdef __cplusplus
}
#endif  
