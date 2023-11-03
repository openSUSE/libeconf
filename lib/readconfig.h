
#pragma once

#include "libeconf.h"
#include "keyfile.h"

extern econf_err readConfigHistoryWithCallback(econf_file ***key_files,
					       size_t *size,
					       const char *dist_conf_dir,
					       const char *run_conf_dir,
					       const char *etc_conf_dir,
					       const char *config_name,
					       const char *config_suffix,
					       const char *delim,
					       const char *comment,
					       char **conf_dirs,
					       const int conf_count,
					       bool (*callback)(const char *filename, const void *data),
					       const void *callback_data);

extern econf_err readConfigWithCallback(econf_file **result,
					const char *dist_conf_dir,
					const char *run_conf_dir,
					const char *etc_conf_dir,
					const char *config_name,
					const char *config_suffix,
					const char *delim,
					const char *comment,
					char **conf_dirs,
					const int conf_count,
					bool (*callback)(const char *filename, const void *data),
					const void *callback_data);
