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

#include <glib.h>
#include <stdio.h>

//merge the contents of two key files
void g_key_file_merge_files(GKeyFile *key_file, const char *file_name, const char *etc_path, const char *usr_path, GError **error) {

  size_t etc_len = strlen(etc_path) + strlen(file_name) + 2;
  char *etc_file = malloc(etc_len);
  snprintf(etc_file, etc_len, "%s/%s", etc_path, file_name);
  size_t usr_len = strlen(usr_path) + strlen(file_name) + 2;
  char *usr_file = malloc(usr_len);
  snprintf(usr_file, usr_len, "%s/%s", usr_path, file_name);

  g_autoptr(GKeyFile) etc_key_file = g_key_file_new();
  if (!g_key_file_load_from_file (etc_key_file, etc_file,
        G_KEY_FILE_KEEP_TRANSLATIONS, error) ||
      !g_key_file_load_from_file (key_file, usr_file,
        G_KEY_FILE_KEEP_TRANSLATIONS, error)) {
    if (!g_error_matches (*error, G_FILE_ERROR, G_FILE_ERROR_NOENT))
      g_error ("Error loading key file: %s", (*error)->message);
    return;
  }

  gsize grp_len;
  g_autofree gchar **groups = g_key_file_get_groups(etc_key_file, &grp_len);

  gsize key_len;
  g_autofree gchar **keys = NULL, *value = NULL;

  for (int i = 0; i < grp_len; i++) {
    keys = g_key_file_get_keys(etc_key_file, groups[i], &key_len, error);
    if (g_error_matches (*error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_GROUP_NOT_FOUND)) {
      g_warning ("Error getting keys: %s", (*error)->message);
      continue;
    }

    for (int j = 0; j < key_len; j++) {
      value = g_key_file_get_value(etc_key_file, groups[i], keys[j], error);
      if (g_error_matches (*error, G_KEY_FILE_ERROR, G_KEY_FILE_ERROR_KEY_NOT_FOUND)) {
        g_warning ("Error getting value: %s", (*error)->message);
        continue;
      }
      g_key_file_set_string(key_file, groups[i], keys[j], value); 
    }
  }
}
