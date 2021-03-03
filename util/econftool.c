/*
  Copyright (C) 2019, 2020 SUSE LLC
  Author: Dominik Gedon <dgedon@suse.de>
          Jorik Cronenberg <jcronenberg@suse.de>

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

#include <errno.h>
#include <ftw.h>
#include <getopt.h>
#include <limits.h>
#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "libeconf.h"

#define UNUSED(x) (void)(x)

static const char *utilname = "econftool";
static bool non_interactive = false;
static char *conf_suffix = NULL; /* the suffix of the filename e.g. .conf */
static char conf_dir[PATH_MAX] = {0}; /* the directory of the config file */
static char conf_basename[PATH_MAX] = {0}; /* the filename without the suffix */
static char conf_filename[PATH_MAX] = {0}; /* the filename including the suffix */
static char conf_path[PATH_MAX] = {0}; /* the path concatenated with the filename and the suffix */
static char xdg_config_dir[PATH_MAX] = {0};
static char root_dir[PATH_MAX] = "/etc";
static char usr_root_dir[PATH_MAX] = "/usr/etc";

/**
 * @brief Shows the usage.
 */
static void usage(void)
{
    fprintf(stderr, "Usage: %s COMMAND [OPTIONS] <filename>.conf\n\n", utilname);
    fprintf(stderr, "COMMANDS:\n");
    fprintf(stderr, "show     reads all snippets for <filename>.conf (in /usr/etc and /etc),\n");
    fprintf(stderr, "         and prints all groups,keys and their values.\n");
    fprintf(stderr, "         The root directories is /. It can be set by the environment\n");
    fprintf(stderr, "         variable $ECONFTOOL_ROOT \n");
    fprintf(stderr, "edit     starts the editor $EDITOR (environment variable) where the\n");
    fprintf(stderr, "         groups, keys and values can be modified and saved afterwards.\n");
    fprintf(stderr, "  -f, --full:      copy the original configuration file to /etc instead of\n");
    fprintf(stderr, "                   creating drop-in files.\n");
    fprintf(stderr, "  --use-home:      only has an effect if the user is root.\n");
    fprintf(stderr, "                   chooses the root home directory instead of /etc.\n");
    fprintf(stderr, "revert   reverts all changes to the vendor versions. Basically deletes\n");
    fprintf(stderr, "         the config file and snippet directory in /etc.\n");
    fprintf(stderr, "  -y, --yes:       assumes yes for all prompts and runs non-interactively.\n\n");
}

/**
 * @brief Change root dir if enviroment variable "ECONFTOOL_ROOT" exists
 *
 * @param change_path the path to be changed
 */
static void change_root_dir(char change_path[PATH_MAX])
{
    if (getenv("ECONFTOOL_ROOT") != NULL) {

        int strlen_econftool_root = strlen(getenv("ECONFTOOL_ROOT"));
        /* check if ECONFTOOL_ROOT has already been added */
        if (strncmp(change_path, getenv("ECONFTOOL_ROOT"), strlen_econftool_root) == 0)
            return;

        if ((strlen(change_path) + strlen_econftool_root) >= PATH_MAX) {
            fprintf(stderr, "ECONFTOOL_ROOT + change_path is too long\n");
            exit(EXIT_FAILURE);
        }

        char *tmp = strdup(change_path);

        strcpy(change_path, getenv("ECONFTOOL_ROOT"));
        strcat(change_path, tmp);

        free(tmp);
    }
}

/**
 * @brief This function is used as a callback for nftw to delete
 *        all files nftw finds.
 */
static int nftw_remove(const char *path, const struct stat *sb, int flag, struct FTW *ftwbuf)
{
    UNUSED(sb);
    UNUSED(flag);
    UNUSED(ftwbuf);
    return remove(path);
}

/**
 * @brief Generate a tmpfile using mkstemp() and write its name into tmp_name
 */
static int generate_tmp_file(char *tmp_name)
{
    int filedes;
    char tmp_filename[] = "/tmp/econftool-XXXXXX";
    filedes = mkstemp(tmp_filename);
    if (filedes == -1) {
        perror("mkstemp() failed");
        return -1;
    }

    strcpy(tmp_name, tmp_filename);
    return 0;
}

/**
 * @brief freeing used memory which has been used for groups
 */
static void free_groups(char ***groups)
{
    econf_free(*groups);
}

/**
 * @brief This command will read all snippets for filename.conf
 *        (econf_readDirs) and print all groups, keys and their
 *        values as an application would see them.
 */
static int econf_show(struct econf_file **key_file)
{
    econf_err econf_error;
    printf ("Vendor config directory: %s\n",usr_root_dir);
    printf ("Config directory for local changes:  %s\n",root_dir);
    printf ("Basename: %s\n",conf_basename);
    printf ("Suffix: %s\n",conf_suffix);
    econf_error = econf_readDirs(key_file, usr_root_dir, root_dir, conf_basename,
                                 conf_suffix, "=", "#");
    if (econf_error) {
        fprintf(stderr, "%d: %s\n", econf_error, econf_errString(econf_error));
        return -1;
    }

    char **groups __attribute__ ((__cleanup__(free_groups))) = NULL;
    char *value = NULL;
    size_t groupCount = 0;

    /* show groups, keys and their value */
    econf_error = econf_getGroups(*key_file, &groupCount, &groups);
    if (econf_error) {
        fprintf(stderr, "%d: %s\n", econf_error, econf_errString(econf_error));
        return -1;
    }
    for (size_t g = 0; g < groupCount; g++) {
        char **keys = NULL;
        size_t key_count = 0;

        econf_error = econf_getKeys(*key_file, groups[g], &key_count, &keys);
        if (econf_error) {
            fprintf(stderr, "%d: %s\n", econf_error, econf_errString(econf_error));
            econf_free(keys);
            return -1;
        }
        printf("%s\n", groups[g]);
        for (size_t k = 0; k < key_count; k++) {
            econf_error = econf_getStringValue(*key_file, groups[g], keys[k], &value);
            if (econf_error) {
                fprintf(stderr, "%d: %s\n", econf_error, econf_errString(econf_error));
                econf_free(keys);
                return -1;
            }
            if (value != NULL)
                printf("%s = %s\n", keys[k], value);
            free(value);
        }
        printf("\n");
        econf_free(keys);
    }

    return 0;
}

/**
 * @brief Generates a tmpfiles from key_file and opens editor to allow user editing.
 *        It then saves the edited in key_file_edit and deletes the tmpfile
 */
static int econf_edit_editor(struct econf_file **key_file_edit, struct econf_file **key_file)
{
    econf_err econf_error;
    int wstatus;
    char *editor;
    char tmpfile_edit[FILENAME_MAX];
    char path_tmpfile_edit[PATH_MAX];
    char tmp_name[PATH_MAX];
    char *tmp_path = getenv("TMPDIR");
    if (tmp_path == NULL)
        tmp_path = "/tmp";

    if (generate_tmp_file(tmp_name))
        return -1;
    snprintf(path_tmpfile_edit, sizeof(path_tmpfile_edit), "%s", tmp_name);
    snprintf(tmpfile_edit, sizeof(tmpfile_edit), "%s", strrchr(path_tmpfile_edit, '/') + 1);

    editor = getenv("EDITOR");
    if (editor == NULL) {
        /* if no editor is specified take vi as default */
        editor = "vi";
    }

    pid_t pid = fork();

    if (pid == -1) {
        perror("fork() failed");
        exit(EXIT_FAILURE);
    } else if (!pid) {
    /* child */

        /* write contents of key_file to tmpfile */
        econf_error = econf_writeFile(*key_file, tmp_path, tmpfile_edit);
        if (econf_error) {
            fprintf(stderr, "%s\n", econf_errString(econf_error));
            exit(EXIT_FAILURE);
        }
        /* execute given command and save result in tmpfile_edit */
        execlp(editor, editor, path_tmpfile_edit, (char *) NULL);
        fprintf(stderr, "Couldn't execute %s: %s\n", editor, strerror(errno));
        exit(EXIT_FAILURE);
    }
    /* parent */
    int ret = 0;
    do {
        if (waitpid(pid, &wstatus, 0) == -1 && errno != EINTR) {
            perror("waitpid");
            ret = -1;
            goto cleanup;
        } else if (!WIFEXITED(wstatus)) {
            fprintf(stderr, "Exited, status: %d\n", WEXITSTATUS(wstatus));
            ret = -1;
            goto cleanup;
        }
    } while (!WIFEXITED(wstatus));

    /* save edits from tmpfile_edit in key_file_edit */
    econf_error = econf_readFile(key_file_edit, path_tmpfile_edit, "=", "#");
    if (econf_error) {
        fprintf(stderr, "%s\n", econf_errString(econf_error));
        ret = -1;
    }

  cleanup:
    remove(path_tmpfile_edit);
    return ret;
}

/**
 * @brief This command will start an editor (EDITOR environment variable),
 *        which shows all groups, keys and their values (like econftool
 *        show output), allows the admin to modify them, and stores the
 *        changes afterwards in a drop-in file in /etc/filename.conf.d/.
 *
 * --full: copy the original config file to /etc instead of creating drop-in
 *         files.
 *
 * If not root, the file will be created in XDG_CONFIG_HOME, which is normally
 * $HOME/.config.
 *
 *  TODO:
 *      - Replace static values of the path with future libeconf API calls
 */
static int econf_edit(struct econf_file **key_file)
{
    econf_err econf_error;
    econf_file *key_file_edit = NULL;

    econf_error = econf_readDirs(key_file, usr_root_dir, root_dir, conf_basename, conf_suffix, "=", "#");

    if (econf_error == ECONF_NOFILE) {
    /* the file does not exist */

        /* create empty key file */
        if ((econf_error = econf_newIniFile(key_file))) {
            fprintf(stderr, "%s\n", econf_errString(econf_error));
            return -1;
        }
    } else if (econf_error != ECONF_SUCCESS) {
        /* other errors besides "missing config file" or "no error" */
        fprintf(stderr, "%s\n", econf_errString(econf_error));
        return -1;
    }

    if (econf_edit_editor(&key_file_edit, key_file)) {
        econf_free(key_file_edit);
        return -1;
    }

    /*********************************************************************/
    /* TODO: analyse the key_files of the 2 tmp files to extract the change
     * - starting with groups, then keys and then values
     */
    int ret = 0;
    char **groups = NULL;
    char **groupsEdit = NULL;
    size_t groupCount = 0;
    size_t groupEditCount = 0;

    /* extract the groups of the original key_file into groups */
    econf_error = econf_getGroups(*key_file, &groupCount, &groups);
    if (econf_error) {
        fprintf(stderr, "%s\n", econf_errString(econf_error));
        ret = -1;
        goto cleanup;
    }
    /* extract the groups of the edited key_file into groupsEdit */
    econf_error = econf_getGroups(key_file_edit, &groupEditCount, &groupsEdit);
    if (econf_error) {
        fprintf(stderr, "%s\n", econf_errString(econf_error));
        ret = -1;
        goto cleanup;
    }

    /* if path does not exist, create it */
    if (access(conf_dir, F_OK) == -1 && errno == ENOENT) {
        if (mkdir(conf_dir, 0755) != 0) {
            perror("mkdir() failed");
            ret = -1;
            goto cleanup;
	}
    }
    /* check if file already exists */
    if (access(conf_path, F_OK) == 0 && !non_interactive) {
        char input[3] = "";
        /* let the user verify that the file should really be overwritten */
        do {
            fprintf(stdout, "The file %s/%s already exists!\n", conf_dir, conf_filename);
            fprintf(stdout, "Do you really want to overwrite it?\nYes [y], no [n]\n");
            if (scanf("%2s", input) != 1) {
                fprintf(stderr, "Didn't read correctly\n");
		strcpy(input, "");
            }
        } while (strcmp(input, "y") != 0 && strcmp(input, "n") != 0);

        if (strcmp(input, "y") == 0) {
            printf( "Writing file %s to %s\n",conf_filename, conf_dir);
            if ((econf_error = econf_writeFile(key_file_edit, conf_dir, conf_filename))) {
                fprintf(stderr, "%s\n", econf_errString(econf_error));
                ret = -1;
                goto cleanup;
            }
        }
    } else {
        printf( "Writing file %s to %s\n",conf_filename, conf_dir);
        if ((econf_error = econf_writeFile(key_file_edit, conf_dir, conf_filename))) {
            fprintf(stderr, "%s\n", econf_errString(econf_error));
            ret = -1;
            goto cleanup;
        }
    }

    /* cleanup */
  cleanup:
    econf_free(key_file_edit);
    econf_free(groupsEdit);
    econf_free(groups);
    return ret;

}

/**
 * @brief Revert all changes to the vendor version. If the user is root,
 *        delete all files in /etc for config file including snippet directory.
 *        If the user is not root delete the config file in xdg_config_dir.
 */
static int econf_revert(bool is_root, bool use_homedir)
{
    char input[3] = "";
    int status = 0;

    if (!is_root || use_homedir) {
        snprintf(conf_path, sizeof(conf_path), "%s/%s", xdg_config_dir, conf_filename);
    } else {
        snprintf(conf_path, sizeof(conf_path), "/etc/%s", conf_filename);
    }

    if (access(conf_path, F_OK) == -1 && (access(conf_dir, F_OK) == -1 || !is_root)) {
        fprintf(stderr, "Could not find %s or a snippet directory\n", conf_path);
        return -1;
    }

    /* Delete config file */
    if (access(conf_path, F_OK) == 0) {
        /* let the user verify that the file should really be deleted */
        while (strcmp(input, "y") != 0 && strcmp(input, "n") != 0
                && !non_interactive) {
            fprintf(stdout, "Delete file %s?\nYes [y], no [n]\n", conf_path);
            if (scanf("%2s", input) != 1) {
                fprintf(stderr, "Didn't read correctly\n");
            }
        }

        if (strcmp(input, "y") == 0 || non_interactive) {
            status = remove(conf_path);
            if (status)
                perror("remove() failed");
            else
                fprintf(stdout, "File %s deleted!\n", conf_path);
        }
    }

    if (!is_root || use_homedir)
        return 0;

    /* Reset input */
    strcpy(input, "");

    /* Delete snippet directory */
    if (access(conf_dir, F_OK) == 0) {
        /* let the user verify that the directory should really be deleted */
        while (strcmp(input, "y") != 0 && strcmp(input, "n") != 0 && !non_interactive) {
            fprintf(stdout, "Delete directory %s?\nYes [y], no [n]\n", conf_dir);
            if (scanf("%2s", input) != 2) {
                fprintf(stderr, "Didn't read correctly\n");
                continue;
            }
        }

        if (strcmp(input, "y") == 0 || non_interactive) {
            status = nftw(conf_dir, nftw_remove, getdtablesize() - 10, FTW_DEPTH | FTW_PHYS);
            if (status)
                perror("nftw() failed");
            else
                fprintf(stdout, "Directory %s deleted!\n", conf_dir);
        }
    }

    return status;
}

int main (int argc, char *argv[])
{
    static const char *dropin_filename = "90_econftool.conf";
    static econf_file *key_file = NULL;
    char home_dir[PATH_MAX] = {0}; /* the path of the home directory */
    bool is_dropin_file = true;
    bool is_root = false;
    bool use_homedir = false;

    /* parse command line arguments. See getopt_long(3) */
    int opt, nonopts;

    int index = 0;
    static struct option longopts[] = {
    /*   name,     arguments,      flag, value */
        {"full",        no_argument,       0, 'f'},
        {"help",        no_argument,       0, 'h'},
        {"yes",         no_argument,       0, 'y'},
        {"use-home",    no_argument,       0, 'u'},
        {0,             0,                 0,  0 }
    };

    while ((opt = getopt_long(argc, argv, "hfy", longopts, &index)) != -1) {
        switch(opt) {
        case 'f':
            /* overwrite path */
            snprintf(conf_dir, sizeof(conf_dir), "%s", "/etc");
            change_root_dir(conf_dir);
            is_dropin_file = false;
            break;
        case 'h':
            usage();
            return EXIT_SUCCESS;
        case 'y':
            non_interactive = true;
            break;
        case 'u':
            use_homedir = true;
            break;
        case '?':
        default:
            fprintf(stderr, "Try '%s --help' for more information.\n", utilname);
            exit(EXIT_FAILURE);
            break;
        }
    }
    nonopts = argc - optind;

    /* only do something if we have an input */
    if (argc < 2) {
        usage();
        return EXIT_FAILURE;
    } else if (argc < 3 || nonopts < 2) {
        fprintf(stderr, "Invalid number of Arguments\n\n");
        usage();
        return EXIT_FAILURE;
    }
    /**** initialization ****/

    /* basic write permission check */
    is_root = getuid() == 0;

    /* get the position of the last dot in the filename to extract
     * the suffix from it.
     */
    conf_suffix = strrchr(argv[optind + 1], '.');

    if (conf_suffix == NULL) {
        fprintf(stderr, "Currently only works with a dot in the filename!\n\n");
        usage();
    }

    /* set filename to the proper argv argument */
    if (strlen(argv[optind + 1]) > sizeof(conf_basename)) {
        fprintf(stderr, "Filename too long\n");
        return EXIT_FAILURE;
    }
    snprintf(conf_basename, strlen(argv[optind + 1]) - strlen(conf_suffix) + 1, "%s", argv[optind + 1]);
    snprintf(conf_filename, sizeof(conf_filename), "%s" , argv[optind + 1]);

    if (is_dropin_file) {
        snprintf(conf_dir, sizeof(conf_dir), "/etc/%s.d", conf_filename);
        change_root_dir(conf_dir);
    }
    snprintf(conf_path, sizeof(conf_path), "%s/%s", conf_dir, conf_filename);

    if (getenv("ECONFTOOL_ROOT") == NULL)
        if (getenv("HOME") != NULL) {
            snprintf(home_dir, sizeof(home_dir), "%s", getenv("HOME"));
        } else {
            struct passwd *pw = getpwuid(getuid());
            if(pw)
                strcpy(home_dir, pw->pw_dir);
        }
    else
        change_root_dir(home_dir);

    if (getenv("XDG_CONFIG_HOME") == NULL) {
        /* if no XDG_CONFIG_HOME is specified take ~/.config as default */
        snprintf(xdg_config_dir, sizeof(xdg_config_dir), "%s/%s", home_dir, ".config");
    } else {
        snprintf(xdg_config_dir, sizeof(xdg_config_dir), "%s", getenv("XDG_CONFIG_HOME"));
    }

    /* Change Root dirs */
    change_root_dir(root_dir);
    change_root_dir(usr_root_dir);

    int ret = 0;

    if (strcmp(argv[optind], "show") == 0) {
        ret = econf_show(&key_file);
    } else if (strcmp(argv[optind], "edit") == 0) {
        if (!is_root || use_homedir) {
            /* adjust path to home directory of the user.*/
            snprintf(conf_dir, sizeof(conf_dir), "%s", xdg_config_dir);
            change_root_dir(conf_dir);
            snprintf(conf_path, sizeof(conf_path), "%s/%s", conf_dir,
                    conf_filename);
        } else if(is_dropin_file) {
            snprintf(conf_filename, sizeof(conf_filename), "%s", dropin_filename);
            snprintf(conf_path, sizeof(conf_path), "%s/%s", conf_dir,
                    conf_filename);
        }

        ret = econf_edit(&key_file);
    } else if (strcmp(argv[optind], "revert") == 0) {
        ret = econf_revert(is_root, use_homedir);
    } else {
        fprintf(stderr, "Unknown command!\n\n");
        usage();
        exit(EXIT_FAILURE);
    }

    /* cleanup */
    econf_free(key_file);
    return ret;
}
