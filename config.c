#include "config.h"
#include "log.h"
#include "soap_instance.h"
#include "address_manager.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define CONFIG_MAX_FILENAME_STRING_LEN 512
#define CONFIG_NAME_XDG "config"
#define CONFIG_NAME_HOME ".voproxyd.conf"
#define CONFIG_NAME_CWD CONFIG_NAME_HOME
#define XDG_DIR_NAME "voproxyd"

struct config g_config;

static int file_exists(char *filename)
{
    struct stat st = { 0 };

    return stat(filename, &st) == 0;
}

static void create_directory_if_not_exists(const char *path)
{
    struct stat st = { 0 };

    if (stat(path, &st) == -1)
        if (mkdir(path, 0700) == -1)
            die(ERR_MKDIR, "Failed to create directory \"%s\": errno = %d", path, errno);
}

static char* get_xdg_filename()
{
    char *xdg_config_home = getenv("XDG_CONFIG_HOME");
    char *home = getenv("HOME");
    char *filename = malloc(CONFIG_MAX_FILENAME_STRING_LEN);

    if (!filename)
        die(ERR_ALLOC, "failed to allocate string buffer");

    if (!home)
        die(ERR_GETENV, "$HOME is not defined");

    if (xdg_config_home)
        snprintf(filename, CONFIG_MAX_FILENAME_STRING_LEN, "%s/%s/%s", xdg_config_home,
                XDG_DIR_NAME, CONFIG_NAME_XDG);
    else
        snprintf(filename, CONFIG_MAX_FILENAME_STRING_LEN, "%s/%s/%s/%s", home, ".config",
                XDG_DIR_NAME, CONFIG_NAME_XDG);

    return filename;
}

static char* get_home_filename()
{
    char *home = getenv("HOME");
    char *filename = malloc(CONFIG_MAX_FILENAME_STRING_LEN);

    if (!filename)
        die(ERR_ALLOC, "failed to allocate string buffer");

    if (!home)
        die(ERR_GETENV, "$HOME is not defined");

    snprintf(filename, CONFIG_MAX_FILENAME_STRING_LEN, "%s/%s", home, CONFIG_NAME_HOME);

    return filename;
}

static char* get_cwd_filename()
{
    char *filename = malloc(CONFIG_MAX_FILENAME_STRING_LEN);

    if (!filename)
        die(ERR_ALLOC, "failed to allocate string buffer");

    if (getcwd(filename, CONFIG_MAX_FILENAME_STRING_LEN) == NULL)
        die(ERR_GETENV, "failed to getcwd");

    strcat(filename, "/" CONFIG_NAME_CWD);

    return filename;
}

static void write_to_xdg_file()
{
    char *filename = get_xdg_filename();
    FILE *f;
    const char contents[] =
        "# this is a check to make sure you've modified the example file\n"
        "unmodified = true # remove this line\n"
        "\n"
        "username = user\n"
        "password = pass\n"
        "\n"
        "# [ports]\n"
        "# 192.168.1.2 = 9002\n"
        "\n"
        "# [192.168.1.2]\n"
        "# profile_idx = 0\n"
        "\n";

    f = fopen(filename, "w+");
    if (!f)
        die(ERR_OPEN, "failed to open file \"%s\"", filename);

    if (fwrite(contents, sizeof(contents) - 1, 1, f) != 1)
        die(ERR_WRITE, "failed to write to file \"%s\"", filename);

    fclose(f);

    free(filename);
}

static void create_xdg_dirs()
{
    char *xdg_config_home = getenv("XDG_CONFIG_HOME");
    char *home = getenv("HOME");
    char *directory;

    if (!home)
        die(ERR_GETENV, "$HOME is not defined");

    directory = malloc(CONFIG_MAX_FILENAME_STRING_LEN);
    if (!directory)
        die(ERR_ALLOC, "failed to allocate string buffer");

    if (xdg_config_home)
        snprintf(directory, CONFIG_MAX_FILENAME_STRING_LEN, "%s", xdg_config_home);
    else
        snprintf(directory, CONFIG_MAX_FILENAME_STRING_LEN, "%s/%s", home, ".config");

    create_directory_if_not_exists(directory);

    if (xdg_config_home)
        snprintf(directory, CONFIG_MAX_FILENAME_STRING_LEN, "%s/%s", xdg_config_home, XDG_DIR_NAME);
    else
        snprintf(directory, CONFIG_MAX_FILENAME_STRING_LEN, "%s/%s/%s", home, ".config", XDG_DIR_NAME);

    create_directory_if_not_exists(directory);

    free(directory);
}

static void create_xdg_file()
{
    create_xdg_dirs();

    write_to_xdg_file();
}

char* config_get_config_filename()
{
    char* xdg = get_xdg_filename();
    char* home = get_home_filename();
    char* cwd = get_cwd_filename();

    if (file_exists(xdg)) {
        free(home);
        free(cwd);
        return xdg;
    }

    if (file_exists(home)) {
        free(xdg);
        free(cwd);
        return home;
    }

    if (file_exists(cwd)) {
        free(xdg);
        free(home);
        return cwd;
    }

    create_xdg_file();

    die(ERR_INVALID_ARGS,
        "error: no config file found. searched paths are (in following order):\n"
        "\n"
        "    $XDG_CONFIG_HOME/%s/%s\n"
        "    $HOME/%s\n"
        "    %s\n"
        "\n"
        "which on your system are:\n"
        "\n"
        "    %s\n"
        "    %s\n"
        "    %s\n"
        "\n"
        "a skeleton config file was created in $XDG_CONFIG_HOME/%s/%s",
        XDG_DIR_NAME,
        CONFIG_NAME_XDG,
        CONFIG_NAME_HOME,
        CONFIG_NAME_CWD,
        xdg,
        home,
        cwd,
        XDG_DIR_NAME,
        CONFIG_NAME_XDG);
}

static int ini_cb(void *user, const char *section, const char *name, const char *value, int line)
{
#define streq(X, Y) (strcmp((X), (Y)) == 0)

    struct soap_instance *instance;

    if (streq(section, "ports")) {
        address_mngr_add_address_by_port(atoi(value), name);
    } else if (!streq(section, "")) { /* ip section */
        instance = address_mngr_find_soap_instance_matching_ip(section);
        if (instance == NULL) {
            log("config file %s:%d warning: can't find ip \"%s\"", (char*)user, line, section);
            return 1;
        }

        if (streq(name, "profile_idx"))
            instance->profile_idx = atoi(value);
        else
            log("config file %s:%d warning: unknown option \"%s\"", (char*)user, line, name);
    }

    if (streq(name, "username"))
        g_config.username = strdup(value);
    else if (streq(name, "password"))
        g_config.password = strdup(value);
    else if (streq(name, "unmodified"))
        die(ERR_CONFIG, "please edit autocreated config and remove line \"unmodified = true\"");
    else
        log("config file %s:%d warning: unknown option \"%s\"", (char*)user, line, name);

    return 1;
}

void config_read()
{
    char *filename = config_get_config_filename();
    const char *template =
        "error in config file %s: no %s provided.\n"
        "remove this file to regenerate example config.\n";

    g_config.username = g_config.password = NULL;

    if (ini_parse(filename, ini_cb, filename) < 0)
        die(ERR_CONFIG, "failed to parse config file \"%s\"", filename);

    if (g_config.username == NULL)
        die(ERR_CONFIG, template, filename, "username");

    if (g_config.password == NULL)
        die(ERR_CONFIG, template, filename, "password");

    free(filename);
}

