#include "config.h"
#include "log.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_STRING_LEN 512
#define CONFIG_NAME_XDG "config"
#define CONFIG_NAME_HOME ".voproxyd.conf"
#define CONFIG_NAME_CWD CONFIG_NAME_HOME
#define XDG_DIR_NAME "voproxyd"

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
    char *filename = malloc(MAX_STRING_LEN);

    if (!filename)
        die(ERR_ALLOC, "failed to allocate string buffer");

    if (!home)
        die(ERR_GETENV, "$HOME is not defined");

    if (xdg_config_home)
        snprintf(filename, MAX_STRING_LEN, "%s/%s/%s", xdg_config_home, XDG_DIR_NAME, CONFIG_NAME_XDG);
    else
        snprintf(filename, MAX_STRING_LEN, "%s/%s/%s/%s", home, ".config", XDG_DIR_NAME, CONFIG_NAME_XDG);

    return filename;
}

static char* get_home_filename()
{
    char *home = getenv("HOME");
    char *filename = malloc(MAX_STRING_LEN);

    if (!filename)
        die(ERR_ALLOC, "failed to allocate string buffer");

    if (!home)
        die(ERR_GETENV, "$HOME is not defined");

    snprintf(filename, MAX_STRING_LEN, "%s/%s", home, CONFIG_NAME_HOME);

    return filename;
}

static char* get_cwd_filename()
{
    char *filename = malloc(MAX_STRING_LEN);

    if (!filename)
        die(ERR_ALLOC, "failed to allocate string buffer");

    if (getcwd(filename, MAX_STRING_LEN) == NULL)
        die(ERR_GETENV, "failed to getcwd");

    strcat(filename, "/" CONFIG_NAME_CWD);

    return filename;
}

static void write_to_xdg_file()
{
    char *filename = get_xdg_filename();
    FILE *f;
    const char contents[] =
        "asdf";

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

    directory = malloc(MAX_STRING_LEN);
    if (!directory)
        die(ERR_ALLOC, "failed to allocate string buffer");

    if (xdg_config_home)
        snprintf(directory, MAX_STRING_LEN, "%s/%s", xdg_config_home, XDG_DIR_NAME);
    else
        snprintf(directory, MAX_STRING_LEN, "%s/%s/%s", home, ".config", XDG_DIR_NAME);

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

void config_read()
{
    char *filename = config_get_config_filename();

    // can i get uhhhhhhhhhh

    free(filename);
}

