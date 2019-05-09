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

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/libeconf.h"

// Process the file of the given file_name and save its contents into key_file
void get_key_file(Key_File **key_file, size_t *key_file_len, const char *file_name, const char delim, const char comment) {
  // LNUM: Base number of key-value pairs in key_file
  // LLEN: Base number of chars in a key, value or group name
  // Once the value of these is exceeded memory space of double the value
  // is allocated
  const size_t LNUM = 16, LLEN = 8;
  size_t lnum = LNUM, llen = LLEN, vlen = 0, file_length = 0;
  char has_delim, ch;

  // Actual number of key-value pairs in file

  // Allocate memory for the Key_File based on LNUM
  Key_File *read_file = malloc(LNUM * sizeof(Key_File));

  // File handle for the given file_name
  FILE *kf = fopen(file_name, "rb");
  if (kf == NULL) {
    errno = ENOENT;
    return;
  }

  // Decide which action to take
  // L: Process next line
  // V: Process next value
  // C: Process next char
  char next_action = 'L', repeat = 1;
  while(repeat) {
    switch(next_action) {
      // Process next line in the key_file if key_file_len exceeds lnum
      // double the allocated memory
      case 'L':
        if (file_length >= lnum) {
          read_file = realloc(read_file, lnum * 2 * sizeof(Key_File));
          lnum*=2;
        }
        read_file[file_length].group = NULL, has_delim = 0;
      // Create a new value
      case 'V':
        vlen = 0, llen = LLEN;
        char *value = malloc(LLEN);
      // Get the next char. If the number of chars in a value exceed llen
      // allocate double the memory. If the character is ' ', '\t' or the line
      // starts with '\n' ignore it.
      case 'C':
        if(!((ch = getc(kf)) == '\n' && vlen == 0)) {
          if (vlen >= llen) {
            value = realloc(value, llen * 2 * sizeof(Key_File));
            llen*=2;
          }
          switch(ch) {
            case '\n':
              value[vlen++] = 0;
              // If a newline char is encountered and the line had no delimiter
              // the line is expected to be a group
              if(!has_delim) {
                read_file[file_length].group = malloc(vlen);
                snprintf(read_file[file_length].group, vlen, value);
                free(value);
                next_action = 'V';
                break;
              } else {
                // If the line is no new group copy the group from the previous line
                if (!(read_file[file_length].group)) {
                  llen = strlen(read_file[file_length - 1].group) + 1;
                  read_file[file_length].group = malloc(llen);
                  snprintf(read_file[file_length].group,
                           llen, read_file[file_length - 1].group);
                }
                // If the line had a delimiter everything after the delimiter is
                // considered to be a value
                read_file[file_length].value = malloc(vlen);
                snprintf(read_file[file_length].value, vlen, value);
                file_length++;
                free(value);
                next_action = 'L';
                break;
              }
            case EOF:
              free(value);
              repeat = 0;
              // Check if the file is really at its end after EOF is encountered.
              if(!feof(kf)) {
                errno = EBADF;
                return;
              }
              break;
            default:
              // If the current char is the delimiter consider the part before to
              // be a key.
              if (ch == delim) {
                has_delim = 1;
                value[vlen++] = 0;
                read_file[file_length].key = malloc(vlen);
                snprintf(read_file[file_length].key, vlen, value);
              // If the line starts with the given comment char ignore the line
              // and proceed with the next
              } else if(ch == comment && vlen == 0) {
                getline(&value, &llen, kf);
              // Default case: append the char to the current value
              } else {
                value[vlen++] = ch;
                next_action = 'C';
                break;
              }
              free(value);
              next_action = 'V';
              break;
          }
        } else {
          next_action = 'C';
        }
    }
  }
  fclose(kf);
  *key_file_len = file_length;
  if(!(*key_file = realloc(read_file, sizeof(Key_File) * file_length)))
    errno = ENOMEM;
}

// Merge the contents of two key files
void merge_key_files(Key_File **merged_key_file, size_t *merge_length, Key_File *usr_file, size_t usr_length, Key_File *etc_file, size_t etc_length) {
  // Maximum length the merged file can have
  size_t max_merge_length = usr_length + etc_length;
  // For each new key in etc_file that matches a group from usr_file
  // usr_offset is increased by one
  size_t usr_offset = 0;
  Key_File *merge_file = malloc(max_merge_length * sizeof(Key_File));
  char new_grp = 1, new_key = 0, check_etc_keys = 0;
  for(int i = 0; i < max_merge_length; i++) {
    for(int j = 0; j < etc_length; j++) {
      // Check for new keys in etc_file
      if(check_etc_keys) {
        new_key = 0;
        for(int k = 0; k < i - new_grp; k++) {
          if(!strcmp(merge_file[k].group, etc_file[j].group)) {
            new_key = 1;
            if(!strcmp(merge_file[k].key, etc_file[j].key)) {
              new_key = 0;
              // If there is no new key found in the last 'j' iteration
              // the merge is complete. Get the merge length and return
              if(j == etc_length - 1) {
                *merge_length = i;
                if(!(*merged_key_file = realloc(merge_file, i * sizeof(Key_File))))
                  errno = ENOMEM;
                return;
              }
              break;
            }
          }
        }
        // New key in etc_file
        if(new_key) {
          if(strcmp(merge_file[i-1].group, etc_file[j].group)) {
            merge_file[i] = merge_file[i-1];
            merge_file[i-1] = etc_file[j];
          } else {
            merge_file[i] = etc_file[j];
          }
          new_key = 0, check_etc_keys = 0;
          usr_offset++;
          break;
        }
        // No new keys for current group found
        if(j == etc_length - 1) {
          check_etc_keys = 0;
          j = -1;
        }
      // If a group, key combination exists in both files take the
      // entry from etc_file
      } else if(i < usr_length + usr_offset) {
        new_key = 1;
        if(!strcmp(usr_file[i - usr_offset].group, etc_file[j].group)) {
          // New value for existing key
          if(!strcmp(usr_file[i - usr_offset].key, etc_file[j].key)) {
            merge_file[i] = etc_file[j];
            new_key = 0, check_etc_keys = 0;
            break;
          }
        }
      // Look for new group entries in etc_file and append them at the end
      } else if(new_grp) {
        new_key = 0;
        for(int k = 0; k < i; k++) {
          if(!strcmp(merge_file[k].group, etc_file[j].group)) {
            new_grp = 0;
            // No new group found
            if(j == etc_length - 1) {
              new_key = 1;
            }
            break;
          }
        }
        // New group found
        if(new_grp) {
          merge_file[i] = etc_file[j];
          break;
        }
        new_grp = 1;
      }
    }
    // If a key exclusively exists in usr_file or etc_file
    if(new_key) {
      // New usr_file key
      if(i < usr_length + usr_offset) {
        merge_file[i] = usr_file[i - usr_offset];
      // If there are no new groups to be found check for new etc keys until
      // there are none left
      } else {
        new_grp = 0;
        // Since in this case there's no new merge_file entry
        // in the current iteration reduce 'i' by one
        i--;
      }
    }
    // If a new group was added trigger check for missing keys
    if((i > 0 && strcmp(merge_file[i].group, merge_file[i-1].group)) || !new_grp) {
      check_etc_keys = 1;
    }
  }
}

// Write content of a Key_File struct to specified location
void write_key_file(Key_File *key_file, size_t key_file_length, const char *save_to_dir, const char *file_name, const char delimiter) {
  // Check if the directory exists
  DIR *dir = opendir(save_to_dir);
  if(dir) {
    closedir(dir);
  } else {
    errno = ENOENT;
    return;
  }
  // Create a file handle for the specified file
  char *save_to = combine_path_name(save_to_dir, file_name);
  FILE *kf = fopen(save_to, "w");
  if(kf == NULL) {
    errno = EPERM;
    return;
  }

  // Write to file
  for(int i = 0; i < key_file_length; i++) {
    if(i == 0 || strcmp(key_file[i-1].group, key_file[i].group)) {
      if(i != 0) fprintf(kf, "\n");
      fprintf(kf, "%s\n", key_file[i].group);
    }
    fprintf(kf, "%s%c%s\n", key_file[i].key, delimiter, key_file[i].value);
  }

  // Clean up
  free(save_to);
  fclose(kf);
}

// Wrapper function to perform the merge in one step
void merge_files(const char *save_to_dir, const char *file_name, const char *etc_path, const char *usr_path, const char delimiter, const char comment) {

  /* --- GET KEY FILES --- */

  char *usr_file_name = combine_path_name(usr_path, file_name);
  char *etc_file_name = combine_path_name(etc_path, file_name);

  size_t etc_length = 0, usr_length = 0;
  Key_File *etc_file = NULL, *usr_file = NULL;
  get_key_file(&etc_file, &etc_length, etc_file_name, delimiter, comment);
  get_key_file(&usr_file, &usr_length, usr_file_name, delimiter, comment);

  /* --- MERGE KEY FILES --- */

  Key_File *merge_file = NULL;
  size_t merge_length = 0;
  merge_key_files(&merge_file, &merge_length, usr_file, usr_length, etc_file, etc_length);

  /* --- WRITE MERGED FILE --- */

  write_key_file(merge_file, merge_length, save_to_dir, file_name, delimiter);

  /* --- CLEAN UP --- */

  destroy(etc_file, etc_length);
  destroy(usr_file, usr_length);

  // Since merge_file consists of pointers to elements of usr_file and
  // etc_file, that have already been cleaned up, free is sufficient here
  free(merge_file);
  free(etc_file_name), etc_file_name = NULL;
  free(usr_file_name), usr_file_name = NULL;
}

// Combine file path and file name
char* combine_path_name(const char *file_path, const char *file_name) {
  size_t combined_len = strlen(file_path) + strlen(file_name) + 2;
  char *combined = malloc(combined_len);
  snprintf(combined, combined_len, "%s/%s", file_path, file_name);
  return combined;
}

// Free memory allocated by key_file
void destroy(Key_File *key_file, size_t length) {
  for(int i = 0; i < length; i++) {
    free(key_file[i].group);
    free(key_file[i].key);
    free(key_file[i].value);
  }
  free(key_file), key_file = NULL;
}
