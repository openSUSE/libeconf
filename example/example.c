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

int main() {

  printf("------------------------ OUTPUT START ------------------------\n");

  clock_t begin = clock();

  //Key_File *key_file = econf_newIniFile();
  //Key_File *key_file = econf_get_key_file("example/etc/example/example.ini", "=", '#');

  Key_File *key_file = econf_get_conf_from_dirs("example/usr/etc", "example/etc",
                                                "example", ".ini", "=", '#', NULL);

  econf_setInt64Value(key_file, "[Basic Types]", "Int", INT64_MAX, NULL);
  printf("Int: %ld\n", econf_getInt64Value(key_file, "[Basic Types]", "Int", NULL));

  econf_setUInt64Value(key_file, "[Basic Types]", "UInt", UINT64_MAX, NULL);
  printf("Unsigned Int: %lu\n", econf_getUInt64Value(key_file, "[Basic Types]", "UInt", NULL));

  econf_setFloatValue(key_file, "[Basic Types]", "Float", M_PI, NULL);
  printf("Float: %.*g\n", 8, econf_getFloatValue(key_file, "[Basic Types]", "Float", NULL));

  econf_setDoubleValue(key_file, "[Basic Types]", "Double", M_PI, NULL);
  printf("Double: %.*g\n", 16, econf_getDoubleValue(key_file, "[Basic Types]", "Double", NULL));

  econf_setStringValue(key_file, "[Basic Types]", "String", "\" Hello World! \"", NULL);
  printf("String: %s\n", econf_getStringValue(key_file, "[Basic Types]", "String", NULL));

  // Square brackets around the group/section are not needed
  econf_setBoolValue(key_file, "Basic Types", "Bool", "YeS", NULL);
  printf("Bool: %d\n", econf_getBoolValue(key_file, "[Basic Types]", "Bool", NULL));

  econf_setValue(key_file, "Basic Types", "Generic", "Any value can go here!", NULL);

  size_t key_number = 0;
  char **keys = econf_getKeys(key_file, "Basic Types", &key_number, NULL);
  printf("Keys: ");
  for (int i = 0; i < key_number; i++) {
    printf("%s, ", keys[i]);
  }
  puts("\n");
  econf_destroy(keys);

  econf_write_key_file(key_file, "example/", "test.ini", NULL);

  econf_destroy(key_file);

  clock_t end = clock();

  printf("Execution time: %fms\n", (double)(end - begin) / CLOCKS_PER_SEC * 1000);

  printf("------------------------- OUTPUT END -------------------------\n");
}
