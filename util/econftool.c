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
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <limits.h>
#include <ftw.h>

#include "libeconf.h"

static const char *utilname = "econftool";
static econf_file *key_file = NULL;
static bool non_interactive = false;
char *suffix = NULL; /* the suffix of the filename e.g. .conf */
char path[PATH_MAX]; /* the path of the config file */
char filename[PATH_MAX]; /* the filename without the suffix */
char filename_suffix[PATH_MAX]; /* the filename with the suffix */
char path_filename[PATH_MAX]; /* the path concatenated with the filename and the suffix */
char root_dir[PATH_MAX] = "/etc";
char usr_root_dir[PATH_MAX] = "/usr/etc";
char xdg_config_dir[PATH_MAX];

/**
 * @brief Shows the usage.
 */

static void usage(void)
{
    fprintf(stderr, "Usage: %s COMMANDS [OPTIONS] filename.conf\n\n", utilname);
    fprintf(stderr, "COMMAND:\n");
    fprintf(stderr, "show     reads all snippets for filename.conf and prints all groups,\n");
    fprintf(stderr, "         keys and their values.\n");
    fprintf(stderr, "edit     starts the editor $EDITOR (environment variable) where the\n");
    fprintf(stderr, "         groups, keys and values can be modified and saved afterwards.\n");
    fprintf(stderr, "  -f, --full:      copy the original configuration file to /etc instead of\n");
    fprintf(stderr, "                   creating drop-in files.\n");
    fprintf(stderr, "  -y, --yes:       Assumes yes for all prompts.\n");
    fprintf(stderr, "revert   reverts all changes to the vendor versions. Basically deletes\n");
    fprintf(stderr, "         the config file and snippet directory in /etc.\n");
    fprintf(stderr, "  -y, --yes:       Assumes yes for all prompts and runs non-interactively.\n\n");
}

/**
 * @brief Change root dir if enviroment variable "ECONFTOOL_ROOT" exists
 *
 * @param path The path to be changed
 */
static void change_root_dir(char path[PATH_MAX])
{
    if (getenv("ECONFTOOL_ROOT") != NULL) {
        if (strlen(path) + strlen(getenv("ECONFTOOL_ROOT")) >= PATH_MAX) {
            fprintf(stderr, "ECONFTOOL_ROOT + path is too long\n");
            exit(EXIT_FAILURE);
        }

        char *tmp = strdup(path);

        strcpy(path, getenv("ECONFTOOL_ROOT"));
        strcat(path, tmp);

        free(tmp);
    }
}

/**
 * @brief This function is used as a callback for nftw to delete
 *        all files nftw finds.
 */
static int nftw_remove(const char *path, const struct stat *sb, int flag, struct FTW *ftwbuf)
{
    return remove(path);
}

/**
 * @brief Generate a tmpfile using mkstemp() and write its name into tmp_name
 */
static int generate_tmp_file(char *tmp_name)
{
    int filedes;
    char name_template[21] = "/tmp/econftool-XXXXXX";
    filedes = mkstemp(name_template);
    if (filedes == -1) {
        perror("mkstemp() failed");
        return -1;
    }

    char fdpath[PATH_MAX];
    char tmp_filename[PATH_MAX];
    snprintf(fdpath, PATH_MAX, "/proc/self/fd/%d", filedes);
    ssize_t r = readlink(fdpath, tmp_filename, PATH_MAX);
    tmp_filename[r] = '\0';
    strcpy(tmp_name, tmp_filename);
    return 0;
}

/**
 * @brief This command will read all snippets for filename.conf
 *        (econf_readDirs) and print all groups, keys and their
 *        values as an application would see them.
 */
static int econf_show(void)
{
    econf_err econf_error;

    econf_error = econf_readDirs(&key_file, usr_root_dir, root_dir, filename, suffix, "=", "#");
    if (econf_error) {
        fprintf(stderr, "%s\n", econf_errString(econf_error));
        econf_free(key_file);
        return -1;
    }

    char **groups = NULL;
    char *value = NULL;
    size_t groupCount = 0;

    /* show groups, keys and their value */
    econf_error = econf_getGroups(key_file, &groupCount, &groups);
    if (econf_error) {
        fprintf(stderr, "%s\n", econf_errString(econf_error));
        econf_free(groups);
        econf_free(key_file);
        return -1;
    }
    for (size_t g = 0; g < groupCount; g++)
    {
        char **keys = NULL;
        size_t key_count = 0;

        econf_error = econf_getKeys(key_file, groups[g], &key_count, &keys);
        if (econf_error)
        {
            fprintf(stderr, "%s\n", econf_errString(econf_error));
            econf_free(groups);
            econf_free(keys);
            return -1;
        }
        printf("%s\n", groups[g]);
        for (size_t k = 0; k < key_count; k++) {
            econf_error = econf_getStringValue(key_file, groups[g], keys[k], &value);
            if (econf_error) {
                fprintf(stderr, "%s\n", econf_errString(econf_error));
                econf_free(groups);
                econf_free(keys);
                free(value);
                return -1;
            }
            if (value != NULL)
                printf("%s = %s\n", keys[k], value);
            free(value);
        }
        printf("\n");
        econf_free(keys);
    }
    econf_free(groups);
    return 0;
}

/**
 * @brief Generates a tmpfiles from key_file and opens editor to allow user editing.
 *        It then saves the edited in key_file_edit and deletes the tmpfile
 */
static int econf_edit_editor(struct econf_file **key_file_edit)
{
    econf_err econf_error;
    int ret = 0;
    int error;
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
        econf_error = econf_writeFile(key_file, tmp_path, tmpfile_edit);
        if (econf_error) {
            fprintf(stderr, "%s\n", econf_errString(econf_error));
            exit(EXIT_FAILURE);
        }
        /* execute given command and save result in tmpfile_edit */
        error = execlp(editor, editor, path_tmpfile_edit, (char *) NULL);

        if (error != 0) {
            fprintf(stderr, "Couldn't open %s! %s\n", editor, strerror(errno));
            exit(EXIT_FAILURE);
        } else {
            exit(EXIT_SUCCESS);
        }
    }
    /* parent */
    if (waitpid(pid, &wstatus, 0) == -1) {
        fprintf(stderr, "Error using waitpid().\n");
        ret = -1;
        goto cleanup;
    } else if (WIFEXITED(wstatus) && WEXITSTATUS(wstatus) != 0) {
        ret = -1;
        goto cleanup;
    }

    /* save edits from tmpfile_edit in key_file_edit */
    econf_error = econf_readFile(&*key_file_edit, path_tmpfile_edit, "=", "#");
    if (econf_error) {
        fprintf(stderr, "%s\n", econf_errString(error));
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
 * If not root, the file will be created in XDG_CONF_HOME, which is normally
 * $HOME/.config.
 *
 *  TODO:
 *      - Replace static values of the path with future libeconf API calls
 */
static int econf_edit(void)
{
    econf_err econf_error;
    int ret = 0;
    econf_file *key_file_edit = NULL;

    econf_error = econf_readDirs(&key_file, usr_root_dir, root_dir, filename, suffix, "=", "#");

    if (econf_error == ECONF_NOFILE) {
    /* the file does not exist */

        /* create empty key file */
        if ((econf_error = econf_newIniFile(&key_file))) {
            fprintf(stderr, "%s\n", econf_errString(econf_error));
            return -1;
        }
    } else if (econf_error != ECONF_SUCCESS) {
        /* other errors besides "missing config file" or "no error" */
        fprintf(stderr, "%s\n", econf_errString(econf_error));
        return -1;
    }

    if (econf_edit_editor(&key_file_edit)) {
        econf_free(key_file_edit);
        return -1;
    }

    /*********************************************************************/
    /* TODO: analyse the key_files of the 2 tmp files to extract the change
     * - starting with groups, then keys and then values
     */
    char **groups = NULL;
    char **groupsEdit = NULL;
    size_t groupCount = 0;
    size_t groupEditCount = 0;

    /* extract the groups of the original key_file into groups */
    econf_error = econf_getGroups(key_file, &groupCount, &groups);
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
    if (access(path, F_OK) == -1 && errno == ENOENT) {
        if (mkdir(path, 0755) != 0) {
            perror("mkdir() failed");
            ret = -1;
            goto cleanup;
        }
    }
    /* check if file already exists */
    if (access(path_filename, F_OK) == 0 && !non_interactive) {
        char input[3] = "";
        /* let the user verify that the file should really be overwritten */
        do {
            fprintf(stdout, "The file %s/%s already exists!\n", path, filename_suffix);
            fprintf(stdout, "Do you really want to overwrite it?\nYes [y], no [n]\n");
            scanf("%2s", input);
        } while (strcmp(input, "y") != 0 && strcmp(input, "n") != 0);

        if (strcmp(input, "y") == 0) {
            if ((econf_error = econf_writeFile(key_file_edit, path, filename_suffix))) {
                fprintf(stderr, "%s\n", econf_errString(econf_error));
                ret = -1;
                goto cleanup;
            }
        }
    } else {
        if ((econf_error = econf_writeFile(key_file_edit, path, filename_suffix))) {
            fprintf(stderr, "%s\n", econf_errString(econf_error));
            econf_free(key_file_edit);
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
 * @biref Revert all changes to the vendor version. If the user is root,
 *        delete all files in /etc for config file including snippet directory.
 *        If the user is not root delete the config file in xdg_config_dir.
 */
static int econf_revert(bool is_root)
{
    char input[3] = "";
    int status = 0;

    if (!is_root) {
        snprintf(path_filename, sizeof(path_filename), "%s/%s", xdg_config_dir, filename_suffix);
    } else {
        snprintf(path_filename, sizeof(path_filename), "/etc/%s", filename_suffix);
    }

    if (access(path_filename, F_OK) == -1 && (access(path, F_OK) == -1 || !is_root)) {
        fprintf(stderr, "Could not find %s or a snippet directory\n", path_filename);
        return -1;
    }

    /* Delete config file */
    if (access(path_filename, F_OK) == 0) {
        /* let the user verify that the file should really be deleted */
        while (strcmp(input, "y") != 0 && strcmp(input, "n") != 0
                && !non_interactive) {
            fprintf(stdout, "Delete file %s?\nYes [y], no [n]\n", path_filename);
            scanf("%2s", input);
        }

        if (strcmp(input, "y") == 0 || non_interactive) {
            status = remove(path_filename);
            if (status)
                perror("remove() failed");
            else
                fprintf(stdout, "File %s deleted!\n", path_filename);
        }
    }

    if (!is_root)
        return 0;

    /* Reset input */
    strcpy(input, "");

    /* Delete snippet directory */
    if (access(path, F_OK) == 0) {
        /* let the user verify that the directory should really be deleted */
        while (strcmp(input, "y") != 0 && strcmp(input, "n") != 0 && !non_interactive) {
            fprintf(stdout, "Delete directory %s?\nYes [y], no [n]\n", path);
            scanf("%2s", input);
        }

        if (strcmp(input, "y") == 0 || non_interactive) {
            status = nftw(path, nftw_remove, getdtablesize() - 10, FTW_DEPTH);
            if (status)
                perror("nftw() failed");
            else
                fprintf(stdout, "Directory %s deleted!\n", path);
        }
    }

    return status;
}

int main (int argc, char *argv[])
{
    static const char *dropin_filename = "90_econftool.conf";
    char home[PATH_MAX]; /* the path of the home directory */
    bool is_dropin_file = true;
    bool is_root = false;
    int ret = 0;
    uid_t uid = getuid();

    memset(path, 0, PATH_MAX);
    memset(home, 0, PATH_MAX);
    memset(filename, 0, PATH_MAX);
    memset(path_filename, 0, PATH_MAX);

    /* parse command line arguments. See getopt_long(3) */
    int opt, nonopts;
    int index = 0;
    static struct option longopts[] = {
    /*   name,     arguments,      flag, value */
        {"full",   no_argument,       0, 'f'},
        {"help",   no_argument,       0, 'h'},
        {"yes",    no_argument,       0, 'y'},
        {0,        0,                 0,  0 }
    };

    while ((opt = getopt_long(argc, argv, "hfy", longopts, &index)) != -1) {
        switch(opt) {
        case 'f':
            /* overwrite path */
            snprintf(path, sizeof(path), "%s", "/etc");
            change_root_dir(path);
            is_dropin_file = false;
            break;
        case 'h':
            usage();
            return EXIT_SUCCESS;
        case 'y':
            non_interactive = true;
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
    is_root = uid == 0;

    /* get the position of the last dot in the filename to extract
     * the suffix from it.
     */
    suffix = strrchr(argv[optind + 1], '.');

    if (suffix == NULL) {
        fprintf(stderr, "Currently only works with a dot in the filename!\n\n");
        usage();
    }

    /* set filename to the proper argv argument */
    if (strlen(argv[optind + 1]) > sizeof(filename)) {
        fprintf(stderr, "Filename too long\n");
        return EXIT_FAILURE;
    }
    snprintf(filename, strlen(argv[optind + 1]) - strlen(suffix) + 1, "%s", argv[optind + 1]);
    snprintf(filename_suffix, sizeof(filename_suffix), argv[optind + 1]);

    if (is_dropin_file) {
        snprintf(path, sizeof(path), "/etc/%s.d", filename_suffix);
        change_root_dir(path);
    }
    snprintf(path_filename, sizeof(path_filename), "%s/%s", path, filename_suffix);

    if (getenv("ECONFTOOL_ROOT") == NULL)
        snprintf(home, sizeof(home), "%s", getenv("HOME"));
    else
        change_root_dir(home);

    if (getenv("XDG_CONFIG_HOME") == NULL) {
        /* if no XDG_CONFIG_HOME is specified take ~/.config as default */
        snprintf(xdg_config_dir, sizeof(xdg_config_dir), "%s/%s", home, ".config");
    } else {
        snprintf(xdg_config_dir, sizeof(xdg_config_dir), "%s", getenv("XDG_CONFIG_HOME"));
    }

    /* Change Root dirs */
    change_root_dir(root_dir);
    change_root_dir(usr_root_dir);


    if (strcmp(argv[optind], "show") == 0) {
        ret = econf_show();
    } else if (strcmp(argv[optind], "edit") == 0) {
        if (!is_root) {
            /* adjust path to home directory of the user.*/
            snprintf(path, sizeof(path), "%s", xdg_config_dir);
            change_root_dir(path);
        } else if(is_dropin_file) {
            snprintf(filename_suffix, sizeof(filename_suffix), "%s", dropin_filename);
            snprintf(path_filename, sizeof(path_filename), "%s/%s", path,
                    filename_suffix);
        }

        ret = econf_edit();
    } else if (strcmp(argv[optind], "revert") == 0) {
        ret = econf_revert(is_root);
    } else {
        fprintf(stderr, "Unknown command!\n");
        exit(EXIT_FAILURE);
    }

    /* cleanup */
    econf_free(key_file);
    return ret;
}