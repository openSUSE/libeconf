/*
  Copyright (C) 2019, 2020 SUSE LLC
  Author: Dominik Gedon <dgedon@suse.de>

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
#include <sys/stat.h>
#include <sys/wait.h>
#include <limits.h>

#include "libeconf.h"

static const char *utilname = "econftool";
static const char *TMPPATH= "/tmp";
static const char *TMPFILE_ORIG = "econftool.tmp";
static const char *TMPFILE_EDIT = "econftool_edits.tmp";
static const char *DROPINFILENAME = "90_econftool.conf";
static bool isRoot = false;
static bool isDropinFile = true;
static econf_file *key_file = NULL;
static econf_file *key_file_edit = NULL;

static void deleteTmpFiles(void);
static void diffGroups(char **, char **, char ***, char ***, char ***, size_t *,
        size_t *, size_t *, size_t *, size_t *);
static void newProcess(const char *, char *, const char *, econf_file *);
static void usage(void);
static void changeRootDir(char *);

int main (int argc, char *argv[])
{
    static const char CONFDIR[] = "/.config";
    econf_err error;
    char *suffix = NULL; /* the suffix of the filename e.g. .conf */
    char *posLastDot;
    char path[PATH_MAX]; /* the path of the config file */
    char home[PATH_MAX]; /* the path of the home directory */
    char filename[PATH_MAX]; /* the filename without the suffix */
    char filenameSuffix[PATH_MAX]; /* the filename with the suffix */
    char pathFilename[PATH_MAX]; /* the path concatenated with the filename and the suffix */
    char rootDir[PATH_MAX] = "/etc";
    char usrRootDir[PATH_MAX] = "/usr/etc";
    uid_t uid = getuid();
    uid_t euid = geteuid();

    memset(path, 0, PATH_MAX);
    memset(home, 0, PATH_MAX);
    memset(filename, 0, PATH_MAX);
    memset(pathFilename, 0, PATH_MAX);

    /* parse command line arguments. See getopt_long(3) */
    int opt;
    int index = 0;
    static struct option longopts[] = {
    /*   name,     arguments,      flag, value */
        {"full",   no_argument,       0, 'f'},
        {"help",   no_argument,       0, 'h'},
        {0,        0,                 0,  0 }
    };

    while ((opt = getopt_long(argc, argv, "hf", longopts, &index)) != -1) {
        switch(opt) {
        case 'f':
            /* overwrite path */
            snprintf(path, sizeof(path), "%s", "/etc");
            changeRootDir(path);
            isDropinFile = false;
            break;
        case 'h':
            usage();
            break;
        case '?':
        default:
            fprintf(stderr, "Try '%s --help' for more information.\n", utilname);
            exit(EXIT_FAILURE);
            break;
        }
    }

    /* only do something if we have an input */
    if (argc < 2)
        usage();
    else if (argc < 3) {
        fprintf(stderr, "Missing filename!\n");
        exit(EXIT_FAILURE);
    } else if (argc > 4) {
        fprintf(stderr, "Too many arguments!\n");
        exit(EXIT_FAILURE);
    } else if (argc == 3 && (strcmp(argv[2], "--full") == 0)) {
        fprintf(stderr, "Missing filename!\n");
        exit(EXIT_FAILURE);
    }

    /**** initialization ****/

    /* basic write permission check */
    if (uid == 0 && uid == euid)
        isRoot = true;
    else
        isRoot = false;

    /* get the position of the last dot in the filename to extract
     * the suffix from it. With edit we have to include the
     * possibility that someone uses the --full option and take
     * that into account when extracting the suffix.
     */
    if (argc == 3)
        posLastDot = strrchr(argv[2], '.');
    else
        posLastDot = strrchr(argv[3], '.');

    if (posLastDot == NULL) {
        fprintf(stderr, "Currently only works with a dot in the filename!\n");
        exit(EXIT_FAILURE);
    }
    suffix = posLastDot;

    /* set filename to the proper argv argument
     * argc == 3 -> edit
     * argc == 4 -> edit --full
     */
    if (argc == 4) {
        if (strlen(argv[3]) > sizeof(filename)) {
            fprintf(stderr, "Filename too long\n");
            return EXIT_FAILURE;
        }
        snprintf(filename, strlen(argv[3]) - strlen(posLastDot) + 1, "%s", argv[3]);
        snprintf(filenameSuffix, sizeof(filenameSuffix), argv[3]);
    } else {
        if (strlen(argv[2]) > sizeof(filename)) {
            fprintf(stderr, "Filename too long\n");
            return EXIT_FAILURE;
        }
        snprintf(filename, strlen(argv[2]) - strlen(posLastDot) + 1, "%s", argv[2]);
        snprintf(filenameSuffix, sizeof(filenameSuffix), argv[2]);
    }

    if (isDropinFile) {
        snprintf(path, sizeof(path), "%s%s%s", "/etc/", filenameSuffix, ".d");
        changeRootDir(path);
    }
    snprintf(pathFilename, sizeof(pathFilename), "%s%s%s", path, "/", filenameSuffix);

    const char *editor = getenv("EDITOR");
    if (editor == NULL) {
        /* if no editor is specified take vi as default */
        editor = "/usr/bin/vi";
    }

    if (getenv("ECONFTOOL_ROOT") == NULL)
        snprintf(home, sizeof(home), "%s", getenv("HOME"));
    else
        changeRootDir(home);

    char *xdgConfigDir = getenv("XDG_CONFIG_HOME");
    if (xdgConfigDir == NULL) {
        /* if no XDG_CONFIG_HOME is specified take ~/.config as
         * default
         */
        strncat(home, CONFDIR, sizeof(home) - strlen(home) - 1);
        xdgConfigDir = home;
    }

    /* Change Root dirs */
    changeRootDir(rootDir);
    changeRootDir(usrRootDir);


    /****************************************************************
     * @brief This command will read all snippets for filename.conf
     *        (econf_readDirs) and print all groups, keys and their
     *        values as an application would see them.
     */
    if (strcmp(argv[optind], "show") == 0) {
        if ((error = econf_readDirs(&key_file, usrRootDir, rootDir, filename, suffix, "=", "#"))) {
            fprintf(stderr, "%s\n", econf_errString(error));
            econf_free(key_file);
            return EXIT_FAILURE;
        }

        char **groups = NULL;
        char *value = NULL;
        size_t group_count = 0;

        /* show groups, keys and their value */
        if ((error = econf_getGroups(key_file, &group_count, &groups))) {
            fprintf(stderr, "%s\n", econf_errString(error));
            econf_free(groups);
            econf_free(key_file);
            return EXIT_FAILURE;
        }
        char **keys = NULL;
        size_t key_count = 0;

        for (size_t g = 0; g < group_count; g++)
        {
            if (error = econf_getKeys(key_file, groups[g], &key_count, &keys))
            {
                fprintf(stderr, "%s\n", econf_errString(error));
                econf_free(groups);
                econf_free(keys);
                econf_free(key_file);
                return EXIT_FAILURE;
            }
            printf("%s\n", groups[g]);
            for (size_t k = 0; k < key_count; k++) {
                if ((error = econf_getStringValue(key_file, groups[g], keys[k], &value))
                    || value == NULL || strlen(value) == 0) {
                    fprintf(stderr, "%s\n", econf_errString(error));
                    econf_free(groups);
                    econf_free(keys);
                    econf_free(key_file);
                    free(value);
                    return EXIT_FAILURE;
                }
                printf("%s = %s\n", keys[k], value);
                free(value);
            }
            printf("\n");
            econf_free(keys);
        }
        econf_free(groups);

    /****************************************************************
     * @brief This command will print the content of the files and the name of the
     *        file in the order as read by econf_readDirs.
     * TODO:
     *        - Enhance the libeconf API first, then implement
     */
    } else if (strcmp(argv[optind], "cat") == 0) {
        fprintf(stderr, "Not implemented yet!\n");

    /****************************************************************
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
    } else if (strcmp(argv[optind], "edit") == 0) {
        if (argc == 4 && (strcmp(argv[optind - 1], "--full") != 0)) {
            fprintf(stderr, "Unknown command!\n");
            exit(EXIT_FAILURE);
        } else if (argc == 3 && (strcmp(argv[optind - 1], "--full") == 0)) {
                fprintf(stderr, "Missing filename!\n");
                exit(EXIT_FAILURE);
        } else {
            error = econf_readDirs(&key_file, usrRootDir, rootDir, filename, suffix, "=", "#");

            if (error == ECONF_NOFILE) {

                /* create empty key file */
                if ((error = econf_newIniFile(&key_file))) {
                    fprintf(stderr, "%s\n", econf_errString(error));
                    econf_free(key_file);
                    return EXIT_FAILURE;
                }
            } else if ((error =! ECONF_NOFILE) || (error != ECONF_SUCCESS)) {
                fprintf(stderr, "%s\n", econf_errString(error));
                econf_free(key_file);
                return EXIT_FAILURE;
            }
            if (!isRoot) {
                /* adjust path to home directory of the user.*/
                snprintf(path, sizeof(path), "%s", xdgConfigDir);
                changeRootDir(path);
            } else {
                if(isDropinFile) {
                    memset(filename, 0, PATH_MAX);
                    snprintf(filenameSuffix, sizeof(filenameSuffix), "%s", DROPINFILENAME);
                }
            }
            /* Open $EDITOR in new process 
             * TODO: This call should be replaced with the content from newProcess()
             */
            newProcess(editor, path, filenameSuffix, key_file);
    }

    /****************************************************************
     * @biref Revert all changes to the vendor version. This means,
     *        delete all files in /etc for this config.
     */
    } else if (strcmp(argv[optind], "revert") == 0) {
        char input[3] = "";

        /* let the user verify 2 times that the file should really be deleted */
        do {
            fprintf(stdout, "Delete file /etc/%s?\nYes [y], no [n]\n", argv[2]);
            scanf("%2s", input);
        } while (strcmp(input, "y") != 0 && strcmp(input, "n") != 0);

        if (strcmp(input, "y") == 0) {
            memset(input, 0, 2);
            do {
                fprintf(stdout, "Do you really wish to delete the file /etc/%s?\n", argv[2]);
                fprintf(stdout, "There is no going back!\nYes [y], no [n]\n");
                scanf("%2s", input);
            } while (strcmp(input, "y") != 0 && strcmp(input, "n") != 0);

            if(strcmp(input, "y") == 0) {
                snprintf(pathFilename, sizeof(pathFilename), "%s%s%s", "/etc/", filename, suffix);

                int status = remove(pathFilename);
                if (status != 0)
                    fprintf(stdout, "%s\n", strerror(errno));
                else
                    fprintf(stdout, "File %s deleted!\n", pathFilename);
            }
        }
    } else {
        fprintf(stderr, "Unknown command!\n");
        exit(EXIT_FAILURE);
    }

    /* cleanup */
    econf_free(key_file);
    deleteTmpFiles();
    return EXIT_SUCCESS;
}

/**
 * @brief Shows the usage.
 */
static void usage(void)
{
    fprintf(stderr, "Usage: %s COMMANDS [OPTIONS] filename.conf\n\n", utilname);
    fprintf(stderr, "COMMANDS:\n");
    fprintf(stderr, "show     reads all snippets for filename.conf and prints all groups,\n");
    fprintf(stderr, "         keys and their values.\n");
    fprintf(stderr, "cat      prints the content and the name of the file in the order as\n");
    fprintf(stderr, "         read by libeconf.\n");
    fprintf(stderr, "edit     starts the editor EDITOR (environment variable) where the\n");
    fprintf(stderr, "         groups, keys and values can be modified and saved afterwards.\n");
    fprintf(stderr, "   --full:   copy the original configuration file to /etc instead of\n");
    fprintf(stderr, "             creating drop-in files.\n");
    fprintf(stderr, "revert   reverts all changes to the vendor versions. Basically deletes\n");
    fprintf(stderr, "         the config file in /etc.\n\n");
    exit(EXIT_FAILURE);
}

/**
 * @brief Deletes the 2 created tmp files in /tmp.
 */
static void deleteTmpFiles(void)
{
    char tmpfile_editedOne[PATH_MAX];
    snprintf(tmpfile_editedOne, PATH_MAX, "%s%s%s", TMPPATH, "/", TMPFILE_ORIG);
    remove(tmpfile_editedOne);

    char tmpfile_editedTwo[PATH_MAX];
    snprintf(tmpfile_editedTwo, PATH_MAX, "%s%s%s", TMPPATH, "/", TMPFILE_EDIT);
    remove(tmpfile_editedTwo);
}

/**
 * @brief Compares two econf_file groups and stores the values in different pointers.
 *
 * This function compares group 1 and group 2 for common, new and missing elements.
 *
 * @par group1  The 1st group to compare.
 * @par group2  The 2nd group to compare.
 * @par new     Elements that are not found in group 1 but in group 2 are stored here.
 * @par deleted Elements that are found in group 1 but not in group 2 are stored here.
 * @par common  Elements that are found in group 1 and group 2 are stored here.
 * @par size_g1 The number of elements in group 1.
 * @par size_g2 The number of elements in group 2.
 * @par size_new The number of elements in new
 * @par size_deleted The number of elements in deleted
 * @par size_common The number of elements in common.
 * @return void.
 */
static void diffGroups(char **group1, char **group2, char ***new, char ***deleted, char ***common,
                       size_t *size_g1, size_t *size_g2, size_t *size_new, size_t *size_deleted,
                       size_t *size_common)
{
    size_t size = *size_g1 + *size_g2 + 1;
    bool found1 = false;
    bool found2 = false;
    *new = calloc(size, sizeof(char*));
    *deleted = calloc(*size_g1 + 1, sizeof(char*));
    *common = calloc(*size_g1 + 1, sizeof(char*));

    if (*new == NULL || *deleted == NULL || *common == NULL) {
        fprintf(stderr, "Error with calloc()!");
        exit(EXIT_FAILURE);
    }
    /* compare group1 with group2 */
    for(size_t i = 0; i < *size_g1; i++) {
        found1 = false;
        for (size_t j = 0; j < *size_g2; j++) {
            if (strcmp(group1[i],group2[j]) == 0) {
                (*common)[*size_common] = strdup(group1[i]);
                (*size_common)++;
                found1 = true;
                break;
            }
        }
        if (!found1) {
            /* inside group1 but not in group2 -> deleted element */
            (*deleted)[*size_deleted] = strdup(group1[i]);
            (*size_deleted)++;
        }
    }
    /* compare group2 with common */
    for (size_t j = 0; j < *size_g2; j++) {
        found2 = false;
        for(size_t i = 0; i < *size_common; i++) {
            if (strcmp(group2[j], (*common)[i]) == 0) {
                found2 = true;
                break;
            }
        }
        if (!found2) {
            /* not in common -> new element */
            (*new)[*size_new] = strdup(group2[j]);
            (*size_new)++;
        }
    }
}

/**
 * @brief  Creates a new process to execute a command and handles
 *         the file saving with the edit command. At the moment
 *         only "econftool edit" uses this function due to a
 *         different approach at the beginning of the project.
 * @param  command The command which should be executed
 * @param  path The constructed path for saving the file later
 * @param  filenameSuffix The filename with suffix
 * @param  key_file the stored config file information
 *
 * TODO:
 *         - Move the code directly into the "econftool edit" section
 *         - Make a diff of the two tmp files to see what actually changed and
 *           write only that change into a drop-in file.
 */
static void newProcess(const char *command, char *path, const char *filenameSuffix,
                       econf_file *key_file)
{
    char pathFilename[PATH_MAX];
    memset(pathFilename, 0, PATH_MAX);
    econf_err error;
    int wstatus = 0;
    pid_t pid = fork();

    if (pid == -1) {
        fprintf(stderr, "fork() failed with: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    } else if (!pid) {
    /* child */

        /* write contents of key_file to 2 temporary files. In the future this
         * will be used to make a diff between the two files and only save the
         * changes the user made.
         */
        if ((error = econf_writeFile(key_file, TMPPATH, TMPFILE_ORIG))) {
            fprintf(stderr, "%s\n", econf_errString(error));
            econf_free(key_file);
            deleteTmpFiles();
            exit(EXIT_FAILURE);
        }
        if ((error = econf_writeFile(key_file, TMPPATH, TMPFILE_EDIT))) {
            fprintf(stderr, "%s\n", econf_errString(error));
            econf_free(key_file);
            deleteTmpFiles();
            exit(EXIT_FAILURE);
        }
        /* combine path and filename of the tmp files and set permission to 600 */
        char combined_tmp1[PATH_MAX];
        memset(combined_tmp1, 0, PATH_MAX);
        snprintf(combined_tmp1, PATH_MAX, "%s%s%s", TMPPATH, "/", TMPFILE_ORIG);

        int perm = chmod(combined_tmp1, S_IRUSR | S_IWUSR);
        if (perm != 0) {
            econf_free(key_file);
            deleteTmpFiles();
            exit(EXIT_FAILURE);
        }
        char path_tmpfile_edit[PATH_MAX];
        memset(path_tmpfile_edit, 0, PATH_MAX);
        snprintf(path_tmpfile_edit, PATH_MAX, "%s%s%s", TMPPATH, "/", TMPFILE_EDIT);

        perm = chmod(path_tmpfile_edit, S_IRUSR | S_IWUSR);
        if (perm != 0) {
            econf_free(key_file);
            deleteTmpFiles();
            exit(EXIT_FAILURE);
        }
        /* execute given command and save result in TMPFILE_EDIT */
        execlp(command, command, path_tmpfile_edit, (char *) NULL);
    } else {
    /* parent */
        if (waitpid(pid, &wstatus, 0) == - 1) {
            fprintf(stderr, "Error using waitpid().\n");
            econf_free(key_file);
            deleteTmpFiles();
            exit(EXIT_FAILURE);
        }

        /* save edits from TMPFILE_EDIT in key_file_edit */
        char tmpfile_edited[PATH_MAX];
        memset(tmpfile_edited, 0, PATH_MAX);
        snprintf(tmpfile_edited, PATH_MAX, "%s%s%s", TMPPATH, "/", TMPFILE_EDIT);

        if ((error = econf_readFile(&key_file_edit, tmpfile_edited, "=", "#"))) {
            fprintf(stderr, "%s\n", econf_errString(error));
            econf_free(key_file);
            econf_free(key_file_edit);
            deleteTmpFiles();
            exit(EXIT_FAILURE);
        }
        /*********************************************************************/
        /* TODO: analyse the key_files of the 2 tmp files to extract the change
         * - starting with groups, then keys and then values
         */
        char **groups = NULL;
        char **groups_edit = NULL;
        char **groups_new = NULL;
        char **groups_common = NULL;
        char **groups_deleted = NULL;
        size_t group_count = 0;
        size_t group_edit_count = 0;
        size_t group_new_count = 0;
        size_t group_deleted_count = 0;
        size_t group_common_count = 0;

        /* extract the groups of the original key_file into groups */
        if ((error = econf_getGroups(key_file, &group_count, &groups))) {
            fprintf(stderr, "%s\n", econf_errString(error));
            econf_free(groups);
            econf_free(key_file);
            exit(EXIT_FAILURE);
        }
        /* extract the groups of the edited key_file into groups_edit */
        if ((error = econf_getGroups(key_file_edit, &group_edit_count, &groups_edit))) {
            fprintf(stderr, "%s\n", econf_errString(error));
            econf_free(groups);
            econf_free(groups_edit);
            econf_free(key_file);
            econf_free(key_file_edit);
            exit(EXIT_FAILURE);
        }

        /* if /etc/filename.conf.d does not exist, create it */
        if (access(path, F_OK) == -1 && errno == ENOENT) {
            int mkDir = mkdir(path,
                    S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH); // 755
            if (mkDir != 0) {
                fprintf(stderr, "-->Error with mkdir()!\n");
                econf_free(key_file);
                deleteTmpFiles();
                exit(EXIT_FAILURE);
            }
        }
        /* check if file already exists */
        snprintf(pathFilename, sizeof(pathFilename), "%s%s%s", path, "/", filenameSuffix);
        if (access(pathFilename, F_OK) == 0) {
            char input[2] = "";
            /* let the user verify that the file should really be overwritten */
            do {
                fprintf(stdout, "The file %s%s%s already exists!\n", path, "/", filenameSuffix);
                fprintf(stdout, "Do you really want to overwrite it?\nYes [y], no [n]\n");
                scanf("%2s", input);
            } while (strcmp(input, "y") != 0 && strcmp(input, "n") != 0);

            if (strcmp(input, "y") == 0) {
                if ((error = econf_writeFile(key_file_edit, path, filenameSuffix))) {
                    fprintf(stderr, "%s\n", econf_errString(error));
                    econf_free(key_file);
                    econf_free(key_file_edit);
                    deleteTmpFiles();
                    exit(EXIT_FAILURE);
                }
            } else {
            /* do nothing */
                econf_free(key_file_edit);
                return;
            }
        } else {
            if ((error = econf_writeFile(key_file_edit, path, filenameSuffix))) {
                fprintf(stderr, "%s\n", econf_errString(error));
                econf_free(key_file);
                econf_free(key_file_edit);
                deleteTmpFiles();
                exit(EXIT_FAILURE);
            }
        }
        /* cleanup */
        econf_free(key_file_edit);
        econf_free(groups_edit);
        econf_free(groups);
        free(groups_new);
        free(groups_deleted);
        free(groups_common);
    }
}

/**
 * @brief Change root dir if enviroment variable "ECONFTOOL_ROOT" exists
 *
 * @param path The path to be changed
 */
void changeRootDir(char *path)
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