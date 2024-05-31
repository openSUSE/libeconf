
#pragma once

#include "libeconf.h"
#include "keyfile.h"

extern econf_err readConfigHistoryWithCallback(econf_file ***key_files,
					       size_t *size,
					       char **parse_dirs,
					       const int parse_dirs_count,
					       const char *config_name,
					       const char *config_suffix,
					       const char *delim,
					       const char *comment,
					       const bool join_same_entries,
					       const bool python_style,
					       char **conf_dirs,
					       const int conf_count,
					       bool (*callback)(const char *filename, const void *data),
					       const void *callback_data);

extern econf_err readConfigWithCallback(econf_file **result,
					const char *config_name,
					const char *config_suffix,
					const char *delim,
					const char *comment,
					char **conf_dirs,
					const int conf_count,
					bool (*callback)(const char *filename, const void *data),
					const void *callback_data);
