/*
  Author: Pascal Arlt <parlt@suse.com>
  Copyright (C) 2019 SUSE Linux GmbH

  Licensed under the GNU Lesser General Public License Version 2.1

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, see
  <http://www.gnu.org/licenses/>.
*/

#include <libeconf.h>

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

int main() {

  printf("------------------------ OUTPUT START ------------------------\n");

  clock_t begin = clock();

  econf_merge_files("example/etc/example/conf.d", "example.ini", "example/etc/example", "example/usr/etc", "=", '#');

  //Key_File *key_file = econf_newIniFile();
  Key_File *key_file = econf_get_key_file("example/etc/example/example.ini", "=", '#');

  econf_setIntValue(key_file, "[Basic Types]", "Int", INT64_MAX);
  printf("Int: %ld\n", econf_getInt64Value(key_file, "[Basic Types]", "Int"));

  econf_setUIntValue(key_file, "[Basic Types]", "UInt", UINT64_MAX);
  printf("Unsigned Int: %lu\n", econf_getUInt64Value(key_file, "[Basic Types]", "UInt"));

  econf_setFloatValue(key_file, "[Basic Types]", "Float", M_PI);
  printf("Float: %.*g\n", 8, econf_getFloatValue(key_file, "[Basic Types]", "Float"));

  econf_setDoubleValue(key_file, "[Basic Types]", "Double", M_PI);
  printf("Double: %.*g\n", 16, econf_getDoubleValue(key_file, "[Basic Types]", "Double"));

  econf_setStringValue(key_file, "[Basic Types]", "String", "\" Hello World! \"");
  printf("String: %s\n", econf_getStringValue(key_file, "[Basic Types]", "String"));

  // Square brackets around the group/section are not needed
  econf_setBoolValue(key_file, "Basic Types", "Bool", "YeS");
  printf("Bool: %d\n", econf_getBoolValue(key_file, "[Basic Types]", "Bool"));

  econf_setValue(key_file, "Basic Types", "Generic", "Any value can go here!");

  econf_write_key_file(key_file, "example/", "test.ini");
  econf_destroy(key_file);

  clock_t end = clock();

  printf("Execution time: %fms\n", (double)(end - begin) / CLOCKS_PER_SEC * 1000);

  printf("------------------------- OUTPUT END -------------------------\n");
}

