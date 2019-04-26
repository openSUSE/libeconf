/*
  Author: Pascal Arlt <parlt@suse.com>
  Copyright (C) 2019 SUSE Linux GmbH

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <glib/gprintf.h>
#include <libeconf.h>

int main() {
  g_autoptr(GKeyFile) key_file = g_key_file_new();
  g_autoptr(GError) error = NULL;

  g_key_file_merge_files(key_file, "example.ini", "example/etc/example", "example/usr/share/defaults", &error);

  g_key_file_save_to_file(key_file, "example/etc/example.conf.d/example.ini", NULL);
  //print the merged key file to stdout
  gchar * merged_key_file;
  merged_key_file = g_key_file_to_data(key_file, NULL, NULL);

  printf("%s\n", merged_key_file);
}
