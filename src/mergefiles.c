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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/keyfile.h"
#include "../include/mergefiles.h"

// Merge contents from existing usr_file groups
// uf: usr_file, ef: etc_file
size_t merge_existing_groups(struct file_entry **fe, Key_File *uf, Key_File *ef) {
  char new_key;
  size_t merge_length = 0, tmp = 0, added_keys = 0;
  for(int i = 0; i <= uf->length; i++) {
    // Check if the group has changed in the last iteration
    if(i == uf->length || (i &&strcmp(uf->file_entry[i].group, uf->file_entry[i-1].group))) {
      for (int j = 0; j < ef->length; j++) {
        // Check for matching groups
        if(!strcmp(uf->file_entry[i-1].group, ef->file_entry[j].group)) {
          new_key = 1;
          for(int k = merge_length; k < i + tmp; k++) {
            // If an existing key is found in ef take the value from ef
            if(!strcmp((*fe)[k].key, ef->file_entry[j].key)) {
              (*fe)[k] = ef->file_entry[j];
              new_key = 0;
              break;
            }
          }
          // If a new key is found for an existing group append it to the group
          if(new_key) (*fe)[i + added_keys++] = ef->file_entry[j];
        }
      }
      merge_length = i + added_keys;
      // Temporary value to reduce amount of iterations in inner for loop
      tmp = added_keys;
    }
    if(i != uf->length) (*fe)[i + added_keys] = uf->file_entry[i];
  }
  return merge_length;
}

// Add entries from etc_file exclusive groups
size_t add_new_groups(struct file_entry **fe, Key_File *uf, Key_File *ef, const size_t merge_length) {
  size_t added_keys = merge_length;
  char new_key;
  for(int i = 0; i < ef->length; i++) {
    new_key = 1;
    for(int j = 0; j < uf->length; j++) {
      if(!strcmp(uf->file_entry[j].group, ef->file_entry[i].group)) {
        new_key = 0;
        break;
      }
    }
    if(new_key) (*fe)[added_keys++] = ef->file_entry[i];
  }
  *fe = realloc(*fe, added_keys * sizeof(struct file_entry));
  return added_keys;
}
