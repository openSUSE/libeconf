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

#include <libeconf.h>

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <inttypes.h>

int main(void) {

  printf("------------------------ OUTPUT START ------------------------\n");

  clock_t begin = clock();

  //econf_file *key_file = econf_newIniFile();
  //econf_file *key_file = econf_readFile("example/etc/example/example.ini", "=", "#");

  econf_file *key_file;

  if (econf_readDirs(&key_file, "example/usr/etc", "example/etc",
				"example", ".ini", "=", "#"))
    return 1; /* XXX better error handling */

  econf_setInt64Value(key_file, "[Basic Types]", "Int", INT64_MAX);
  int64_t i64val;
  econf_getInt64Value(key_file, "[Basic Types]", "Int", &i64val);
  printf("Int: %" PRId64 "\n", i64val);


  econf_setUInt64Value(key_file, "[Basic Types]", "UInt", UINT64_MAX);
  uint64_t u64val;
  econf_getUInt64Value(key_file, "[Basic Types]", "UInt", &u64val);
  printf("Unsigned Int: %" PRIu64 "\n", u64val);

  econf_setFloatValue(key_file, "[Basic Types]", "Float", M_PI);
  float fval;
  econf_getFloatValue(key_file, "[Basic Types]", "Float", &fval);
  printf("Float: %.*g\n", 8, fval);

  econf_setDoubleValue(key_file, "[Basic Types]", "Double", M_PI);
  double dval;
  econf_getDoubleValue(key_file, "[Basic Types]", "Double", &dval);
  printf("Double: %.*g\n", 16, dval);

  econf_setStringValue(key_file, "[Basic Types]", "String", "\" Hello World! \"");
  char *string;
  econf_getStringValue(key_file, "[Basic Types]", "String", &string);
  printf("String: %s\n", string);
  free(string);

  // Square brackets around the group/section are not needed
  econf_setBoolValue(key_file, "Basic Types", "Bool", "YeS");
  bool bval;
  econf_getBoolValue(key_file, "[Basic Types]", "Bool", &bval);
  printf("Bool: %d\n", bval);

  econf_setValue(key_file, "Basic Types", "Generic", "Any value can go here!");

  size_t key_number = 0;
  char **keys;
  if (econf_getKeys(key_file, "Basic Types", &key_number, &keys))
    return 1; /* XXX better error handling */
  printf("Keys: ");
  for (size_t i = 0; i < key_number; i++) {
    printf("%s, ", keys[i]);
  }
  puts("\n");
  econf_free(keys);

  econf_writeFile(key_file, "example/", "test.ini");

  econf_free(key_file);

  clock_t end = clock();

  printf("Execution time: %fms\n", (double)(end - begin) / CLOCKS_PER_SEC * 1000);

  printf("------------------------- OUTPUT END -------------------------\n");
}
