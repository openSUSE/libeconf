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


struct Key_File {
  struct file_entry {
    char *group, *key, *value;
  } *file_entry;
  size_t length;
  char delimiter, comment;
};

void new_kf_value(char **value, size_t *vlen, size_t *llen, const size_t LLEN);
void new_kf_line(struct file_entry **fe, size_t *file_length, size_t *lnum);
Key_File fill_key_file(Key_File read_file, FILE *kf);

// Process the file of the given file_name and save its contents into key_file
Key_File get_key_file(const char *file_name, const char delim, const char comment) {
  Key_File read_file = {.delimiter = delim, .comment = comment};

  // File handle for the given file_name
  FILE *kf = fopen(file_name, "rb");
  if (kf == NULL) {
    read_file.length = -ENOENT;
    return read_file;
  }

  read_file = fill_key_file(read_file, kf);

  fclose(kf);
  return read_file;
}

// Merge the contents of two key files
Key_File merge_key_files(Key_File *usr_file, Key_File *etc_file) {
  // Maximum length the merged file can have
  size_t max_merge_length = usr_file->length + etc_file->length;
  // For each new key in etc_file that matches a group from usr_file
  // usr_offset is increased by one
  size_t usr_offset = 0;
  Key_File merge_file = {.delimiter = etc_file->delimiter, .comment = etc_file->comment};
  merge_file.file_entry = malloc(max_merge_length * sizeof(struct file_entry));
  char new_grp = 1, new_key = 0, check_etc_keys = 0;
  for(int i = 0; i < max_merge_length; i++) {
    for(int j = 0; j < etc_file->length; j++) {
      // Check for new keys in etc_file
      if(check_etc_keys) {
        new_key = 0;
        for(int k = 0; k < i - new_grp; k++) {
          if(!strcmp(merge_file.file_entry[k].group, etc_file->file_entry[j].group)) {
            new_key = 1;
            if(!strcmp(merge_file.file_entry[k].key, etc_file->file_entry[j].key)) {
              new_key = 0;
              // If there is no new key found in the last 'j' iteration
              // the merge is complete. Get the merge length and return
              if(j == etc_file->length - 1) {
                merge_file.length = i;
                merge_file.file_entry = realloc(merge_file.file_entry, i * sizeof(struct file_entry));
                return merge_file;
              }
              break;
            }
          }
        }
        // New key in etc_file
        if(new_key) {
          if(strcmp(merge_file.file_entry[i-1].group, etc_file->file_entry[j].group)) {
            merge_file.file_entry[i] = merge_file.file_entry[i-1];
            merge_file.file_entry[i-1] = etc_file->file_entry[j];
          } else {
            merge_file.file_entry[i] = etc_file->file_entry[j];
          }
          new_key = 0, check_etc_keys = 0;
          usr_offset++;
          break;
        }
        // No new keys for current group found
        if(j == etc_file->length - 1) {
          check_etc_keys = 0;
          j = -1;
        }
      // If a group, key combination exists in both files take the
      // entry from etc_file
      } else if(i < usr_file->length + usr_offset) {
        new_key = 1;
        if(!strcmp(usr_file->file_entry[i - usr_offset].group, etc_file->file_entry[j].group)) {
          // New value for existing key
          if(!strcmp(usr_file->file_entry[i - usr_offset].key, etc_file->file_entry[j].key)) {
            merge_file.file_entry[i] = etc_file->file_entry[j];
            new_key = 0, check_etc_keys = 0;
            break;
          }
        }
      // Look for new group entries in etc_file and append them at the end
      } else if(new_grp) {
        new_key = 0;
        for(int k = 0; k < i; k++) {
          if(!strcmp(merge_file.file_entry[k].group, etc_file->file_entry[j].group)) {
            new_grp = 0;
            // No new group found
            if(j == etc_file->length - 1) {
              new_key = 1;
            }
            break;
          }
        }
        // New group found
        if(new_grp) {
          merge_file.file_entry[i] = etc_file->file_entry[j];
          break;
        }
        new_grp = 1;
      }
    }
    // If a key exclusively exists in usr_file or etc_file
    if(new_key) {
      // New usr_file key
      if(i < usr_file->length + usr_offset) {
        merge_file.file_entry[i] = usr_file->file_entry[i - usr_offset];
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
    if((i > 0 && strcmp(merge_file.file_entry[i].group, merge_file.file_entry[i-1].group)) || !new_grp) {
      check_etc_keys = 1;
    }
  }
}

// Write content of a Key_File struct to specified location
void write_key_file(Key_File key_file, const char *save_to_dir, const char *file_name, const char delimiter) {
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
  for(int i = 0; i < key_file.length; i++) {
    if(i == 0 || strcmp(key_file.file_entry[i-1].group, key_file.file_entry[i].group)) {
      if(i != 0) fprintf(kf, "\n");
      fprintf(kf, "%s\n", key_file.file_entry[i].group);
    }
    fprintf(kf, "%s%c%s\n", key_file.file_entry[i].key, delimiter, key_file.file_entry[i].value);
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

  Key_File usr_file = get_key_file(usr_file_name, delimiter, comment);
  Key_File etc_file = get_key_file(etc_file_name, delimiter, comment);


  /* --- MERGE KEY FILES --- */

  Key_File merged_file = merge_key_files(&usr_file, &etc_file);

  /* --- WRITE MERGED FILE --- */

  write_key_file(merged_file, save_to_dir, file_name, delimiter);

  /* --- CLEAN UP --- */
  free(etc_file_name), etc_file_name = NULL;
  free(usr_file_name), usr_file_name = NULL;
  destroy(usr_file);
  destroy(etc_file);
  free(merged_file.file_entry);
}

// Combine file path and file name
char* combine_path_name(const char *file_path, const char *file_name) {
  size_t combined_len = strlen(file_path) + strlen(file_name) + 2;
  char *combined = malloc(combined_len);
  snprintf(combined, combined_len, "%s/%s", file_path, file_name);
  return combined;
}

// Free memory allocated by key_file
void destroy(Key_File key_file) {
  for(int i = 0; i < key_file.length; i++) {
    free(key_file.file_entry[i].group);
    free(key_file.file_entry[i].key);
    free(key_file.file_entry[i].value);
  }
  free(key_file.file_entry);
}

// Fill the Key File struct with values from the given file handle
Key_File fill_key_file(Key_File read_file, FILE *kf) {
  // LNUM: Base number of key-value pairs in key_file
  // LLEN: Base number of chars in a key, value or group name
  // Once the value of these is exceeded memory space of double the value
  // is allocated
  const size_t LLEN = 8, LNUM = 16;
  size_t file_length = 0, lnum = LNUM, llen = LLEN, vlen = 0;
  char ch, has_delim = 0;
  // Allocate memory for the Key_File based on LNUM
  struct file_entry *fe = malloc(LNUM * sizeof(struct file_entry));
  char *value = malloc(LLEN);

  while((ch = getc(kf)) != EOF) {
    if (vlen >= llen) {
      value = realloc(value, llen * 2);
      llen*=2;
    }
    if (ch == '\n') {
      if(vlen == 0) continue;
      value[vlen++] = 0;
      // If a newline char is encountered and the line had no delimiter
      // the line is expected to be a group
      if(!has_delim) {
        fe[file_length].group = malloc(vlen);
        snprintf(fe[file_length].group, vlen, value);
      } else {
        // If the line is no new group copy the group from the previous line
        if (!(fe[file_length].group)) {
          llen = strlen(fe[file_length - 1].group) + 1;
          fe[file_length].group = malloc(llen);
          snprintf(fe[file_length].group, llen, fe[file_length - 1].group);
        }
        // If the line had a delimiter everything after the delimiter is
        // considered to be a value
        fe[file_length].value = malloc(vlen);
        snprintf(fe[file_length].value, vlen, value);
        new_kf_line(&fe, &file_length, &lnum);
        has_delim = 0;
      }
    // If the current char is the delimiter consider the part before to
    // be a key.
    } else if (ch == read_file.delimiter) {
      has_delim = 1;
      value[vlen++] = 0;
      fe[file_length].key = malloc(vlen);
      snprintf(fe[file_length].key, vlen, value);
      // If the line starts with the given comment char ignore the line
      // and proceed with the next
    } else if(ch == read_file.comment && vlen == 0) {
      getline(&value, &llen, kf);
      // Default case: append the char to the current value
    } else {
      value[vlen++] = ch;
      continue;
    }
    vlen = 0;
  }
  free(value);
  // Check if the file is really at its end after EOF is encountered.
  if(!feof(kf)) {
    read_file.length = -EBADF;
    return read_file;
  }
  read_file.length = file_length;
  fe = realloc(fe, file_length * sizeof(struct file_entry));
  read_file.file_entry = fe;
  return read_file;
}

// Check whether the key file has enough memory allocated, if not realloc
void new_kf_line(struct file_entry **fe, size_t *file_length, size_t *lnum) {
  if (++(*file_length) >= *lnum) {
    *fe = realloc(*fe, *lnum * 2 * sizeof(struct file_entry));
    (*lnum)*=2;
  }
  (*fe)[*file_length].group = NULL;
}
