/*
  Copyright (C) 2019 LLC
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

/* --- defines.h --- */

#include "helpers.h"

/* Defines default values used in the library  */

/* Default econf_file length on creation */
#define KEY_FILE_DEFAULT_LENGTH 8

/* NULL value */
#define KEY_FILE_NULL_VALUE "_none_"
#define KEY_FILE_NULL_VALUE_HASH hashstring(KEY_FILE_NULL_VALUE)

/* Bool values */
#define YES hashstring("yes")
#define NO hashstring("no")
#define TRUE hashstring("true")
#define FALSE hashstring("false")
