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
Key_File fill_key_file(Key_File read_file, FILE *kf);
void end_of_line(struct file_entry **fe, size_t *len, size_t *lnum,size_t vlen, char *buffer);
void new_kf_line(struct file_entry **fe, size_t *file_length, size_t *lnum);
size_t merge_existing_groups(struct file_entry **fe, Key_File *uf, Key_File *ef);
size_t add_new_groups(struct file_entry **fe, Key_File *uf, Key_File *ef, const size_t merge_length);
char* combine_path_name(const char *file_path, const char *file_name);

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
  Key_File merge_file = {.delimiter = usr_file->delimiter, .comment = usr_file->comment};
  struct file_entry *fe = malloc((etc_file->length + etc_file->length)
                                 * sizeof(struct file_entry));

  size_t merge_length = merge_existing_groups(&fe, usr_file, etc_file);
  merge_file.length = add_new_groups(&fe, usr_file, etc_file, merge_length);

  merge_file.file_entry = fe;
  return merge_file;
}

// Write content of a Key_File struct to specified location
void write_key_file(Key_File key_file, const char *save_to_dir, const char *file_name) {
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
    if(!i || strcmp(key_file.file_entry[i-1].group, key_file.file_entry[i].group)) {
      if(i) fprintf(kf, "\n");
      if(key_file.file_entry[i].group != "")
        fprintf(kf, "%s\n", key_file.file_entry[i].group);
    }
    fprintf(kf, "%s%c%s\n", key_file.file_entry[i].key,
            key_file.delimiter, key_file.file_entry[i].value);
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

  write_key_file(merged_file, save_to_dir, file_name);

  /* --- CLEAN UP --- */
  free(etc_file_name);
  free(usr_file_name);
  destroy(usr_file);
  destroy(etc_file);
  free(merged_file.file_entry);
}

/* --- GET KEY FILE HELPERS --- */

// Fill the Key File struct with values from the given file handle
Key_File fill_key_file(Key_File read_file, FILE *kf) {
  // LNUM: Base number of key-value pairs in key_file
  // LLEN: Base number of chars in a key, value or group name
  // Once the value of these is exceeded memory space of double the value
  // is allocated
  const size_t LLEN = 8, LNUM = 16;
  size_t file_length = 0, lnum = LNUM, llen = LLEN, vlen = 0;
  char ch;
  // Allocate memory for the Key_File based on LNUM
  struct file_entry *fe = malloc(LNUM * sizeof(struct file_entry));
  fe->group = "", fe->key = "";
  char *buffer = malloc(LLEN);

  while((ch = getc(kf)) != EOF) {
    if (vlen >= llen) {
      buffer = realloc(buffer, llen * 2);
      llen*=2;
    }
    if (ch == '\n') {
      if(vlen == 0) continue;
      end_of_line(&fe, &file_length, &lnum, vlen, buffer);
    // If the current char is the delimiter consider the part before to
    // be a key.
    } else if (ch == read_file.delimiter) {
      buffer[vlen++] = 0;
      fe[file_length].key = malloc(vlen);
      snprintf(fe[file_length].key, vlen, buffer);
      // If the line starts with the given comment char ignore the line
      // and proceed with the next
    } else if(ch == read_file.comment && vlen == 0) {
      getline(&buffer, &llen, kf);
    // Default case: append the char to the buffer
    } else {
      buffer[vlen++] = ch;
      continue;
    }
    vlen = 0;
  }
  free(buffer);
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

// Write the group/value entry to the given file_entry
void end_of_line(struct file_entry **fe, size_t *len, size_t *lnum, size_t vlen, char *buffer) {
  // Remove potential whitespaces from the end
  while(buffer[vlen-1] == ' ' || buffer[vlen-1] == '\n') vlen--;
  buffer[vlen++] = 0;
  // If a newline char is encountered and the line had no delimiter
  // the line is expected to be a group
  // In this case key is not set
  if((*fe)[*len].key == "") {
    (*fe)[*len].group = malloc(vlen);
    snprintf((*fe)[*len].group, vlen, buffer);
  } else {
    // If the line is no new group copy the group from the previous line
    if (*len && (*fe)[*len].group == "" && (*fe)[*len - 1].group != "") {
      size_t tmp = strlen((*fe)[*len - 1].group) + 1;
      (*fe)[*len].group = malloc(tmp);
      snprintf((*fe)[*len].group, tmp, (*fe)[*len - 1].group);
    }
    // If the line had a delimiter everything after the delimiter is
    // considered to be a value
    (*fe)[*len].value = malloc(vlen);
    snprintf((*fe)[*len].value, vlen, buffer);
    // Perform memory check and increase len by one
    new_kf_line(fe, len, lnum);
  }
}

// Check whether the key file has enough memory allocated, if not realloc
void new_kf_line(struct file_entry **fe, size_t *file_length, size_t *lnum) {
  if (++(*file_length) >= *lnum) {
    *fe = realloc(*fe, *lnum * 2 * sizeof(struct file_entry));
    (*lnum)*=2;
  }
  (*fe)[*file_length].group = "";
  (*fe)[*file_length].key = "";
}

/* --- MERGE HELPERS --- */

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

/* --- GENERAL HELPERS --- */

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
    if(key_file.file_entry[i].group != "")
      free(key_file.file_entry[i].group);
    free(key_file.file_entry[i].key);
    free(key_file.file_entry[i].value);
  }
  free(key_file.file_entry);
}
