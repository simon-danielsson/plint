/*
   ctmp : A project template for C powered by nob.h.

   Source(s):
   https://github.com/simon-danielsson/ctmp/
   https://github.com/tsoding/nob.h

   License information @ EOF.
   */

#define NOB_IMPLEMENTATION
#include "nob.h"
#include "src/main.h"

#define PROJ_NAME "plint"

typedef struct {
    char *project;
    char *description;
    char *author;
    char *contact;
    char *website;
    char *git_tag;
    char *git_hash;
    char *git_repo_url;
    char *c_standard;
} EnvVars;

global_var EnvVars env_variables = (EnvVars){
    .project = PROJ_NAME,
    .description = "This is a new C project!",
    .author = "Simon Danielsson",
    .contact = "contact@simondanielsson.se",
    .website = "https://simondanielsson.se/",
    .c_standard = "c99",
    .git_repo_url = "https://github.com/simon-danielsson/" PROJ_NAME,
    .git_tag = NULL,  // added dynamically
    .git_hash = NULL, // added dynamically
};

global_var char *compilation_flags[] = {
    "-O0",
    "-DDEBUG",
    "-fsanitize=address",
    "-fsanitize=undefined",
    "-fno-omit-frame-pointer",
    "-Wall",
    "-Wpedantic",
    "-Wshadow",
    "-Werror=format-security",
};

global_var char *compilation_flags_release[] = {"-flto", "-O2", "-DNDEBUG"};

// definitions ----------------------------------------------------------------

#define DIR_BUILD "./build"
#define DIR_BUILD_DEBUG "./build/debug/"
#define DIR_BUILD_RELEASE "./build/release/"
#define DIR_SRC "./src"
#define DIR_STATIC "./src/static"
#define DIR_TESTS "./tests"

#define MAX_SRC_FILES 256
global_var char *src_files[MAX_SRC_FILES];
global_var size_t src_files_count = 0;
global_var char *static_files[MAX_SRC_FILES];
global_var size_t static_files_count = 0;

// helper: embed_static(const char *path)
char *format_path_to_name(const char *filename);
void embed_static(const char *path);
bool collect_src_files(Nob_Walk_Entry entry);
bool collect_static_files(Nob_Walk_Entry entry);
intern_fn void print_help();
void append_env_variables(Nob_Cmd *cmd);
#define MAX_ENV_VALUE_LEN 256
void append_env_var(Nob_Cmd *cmd, const char *prefix, const char *value,
        bool c_standard);

typedef enum {
    ARG_BUILD,
    ARG_PRG,
} ArgParserState;

typedef enum {
    NORMAL,
    F_HELP,
    F_VERBOSE,
    F_NO_RUN,
    C_TEST,
    C_RELEASE,
    C_EMBED
} ArgKind;

intern_fn void get_git_details(ArgKind build_kind);

typedef struct {
    ArgKind kind;
    char *str;
    char *str_alt;
    char *descr;
} Arg;

global_var Arg arguments[] = {
    [F_HELP] = (Arg){.kind = F_HELP,
        .str = "-h",
        .str_alt = "--help",
        .descr = "Display help."},
    [F_VERBOSE] = (Arg){.kind = F_VERBOSE,
        .str = "-v",
        .str_alt = "--verbose",
        .descr = "Enable verbose output."},
    [F_NO_RUN] = (Arg){.kind = F_NO_RUN,
        .str = "-n",
        .str_alt = "--no-run",
        .descr = "Don't run binary after compilation."},
    [C_TEST] = (Arg){.kind = C_TEST,
        .str = "test",
        .str_alt = NULL,
        .descr = "Run test(s) in 'tests.c'."},
    [C_RELEASE] = (Arg){.kind = C_RELEASE,
        .str = "release",
        .str_alt = NULL,
        .descr = "Compile and run a release build."},
    [C_EMBED] =
        (Arg){.kind = C_EMBED,
            .str = "embed",
            .str_alt = NULL,
            .descr =
                "Embed content of files in 'src/static' into header files."},
};

bool nob_no_echo = true;

// program --------------------------------------------------------------------

int main(int argc, char **argv) {
    // bool nob_no_echo = false;

    char *prg_args[24] = {0};
    size_t prg_args_count = 0;

    char *build_args[24] = {0};
    size_t build_args_count = 0;

    char bin_name[256] = {0};
    ArgKind build_type = NORMAL;
    bool no_run = false;

    {
        uint i = 0;
        ArgParserState aps = ARG_BUILD;
        while (argv[i]) {
            if (aps == ARG_PRG) {
                prg_args[prg_args_count++] = strdup(argv[i]);
                i++;
                continue;
            } else if (strcmp(argv[i], arguments[C_TEST].str) == 0) {
                build_type = NORMAL;
                build_args[build_args_count++] = "-DTEST";
                break;
            } else if (strcmp(argv[i], arguments[C_RELEASE].str) == 0) {
                build_type = C_RELEASE;
            } else if (strcmp(argv[i], arguments[C_EMBED].str) == 0) {
                build_type = C_EMBED;
            } else if (strcmp(argv[i], arguments[F_HELP].str) == 0 ||
                    strcmp(argv[i], arguments[F_HELP].str_alt) == 0) {
                print_help();
                return 0;
            } else if (strcmp(argv[i], arguments[F_VERBOSE].str) == 0 ||
                    strcmp(argv[i], arguments[F_VERBOSE].str_alt) == 0) {
                nob_no_echo = false;

            } else if (strcmp(argv[i], arguments[F_NO_RUN].str) == 0 ||
                    strcmp(argv[i], arguments[F_NO_RUN].str_alt) == 0) {
                no_run = true;
            } else if (strcmp(argv[i], "--") == 0) {
                aps = ARG_PRG;
                i++;
                continue;
            }

            i++;
        }
    }

    // collect and embed static files
    if (build_type == C_EMBED) {
        if (!nob_mkdir_if_not_exists(DIR_STATIC))
            return 1;
        if (!nob_walk_dir(DIR_STATIC, collect_static_files))
            return 1;
        if (static_files_count > 0) {
            for (size_t i = 0; i < static_files_count; i++)
                embed_static(static_files[i]);
        }
        return 0;
    }

    GO_REBUILD_URSELF(argc, argv);

    if (!nob_mkdir_if_not_exists(DIR_BUILD))
        return 1;
    if (!nob_mkdir_if_not_exists(DIR_BUILD_DEBUG) && build_type == NORMAL)
        return 1;
    if (!nob_mkdir_if_not_exists(DIR_BUILD_RELEASE) && build_type == C_RELEASE)
        return 1;

    // collect '.c' files
    {
        if (!nob_walk_dir(DIR_SRC, collect_src_files))
            return 1;
        if (!nob_walk_dir(DIR_TESTS, collect_src_files))
            return 1;
    }

    // build
    {
        Nob_Cmd cmd = {0};

        nob_cc(&cmd);

        for (size_t i = 0; i < build_args_count; i++) {
            nob_cmd_append(&cmd, build_args[i]);
        }

        {
            get_git_details(build_type);

            append_env_var(&cmd, "-DENV_PROJECT", env_variables.project, false);
            append_env_var(&cmd, "-DENV_DESCR", env_variables.description, false);
            append_env_var(&cmd, "-DENV_AUTHOR", env_variables.author, false);
            append_env_var(&cmd, "-DENV_CONTACT", env_variables.contact, false);
            append_env_var(&cmd, "-DENV_WEBSITE", env_variables.website, false);
            append_env_var(&cmd, "-DENV_GIT_TAG", env_variables.git_tag, false);
            append_env_var(&cmd, "-DENV_GIT_HASH", env_variables.git_hash, false);
            append_env_var(&cmd, "-DENV_GIT_REPO_URL", env_variables.git_repo_url,
                    false);
            append_env_var(&cmd, "-std", env_variables.c_standard, true);
        }

        const char *build_folder;
        if (build_type != C_RELEASE) {
            for (size_t i = 0;
                    i < sizeof(compilation_flags) / sizeof(compilation_flags[0]); i++) {
                nob_cmd_append(&cmd, compilation_flags[i]);
                build_folder = DIR_BUILD_DEBUG;
            }
        } else {
            for (size_t i = 0; i < sizeof(compilation_flags_release) /
                    sizeof(compilation_flags_release[0]);
                    i++) {
                nob_cmd_append(&cmd, compilation_flags_release[i]);
                build_folder = DIR_BUILD_RELEASE;
            }
        }

        snprintf(bin_name, 256, "%s%s-%.7s-%s", build_folder, PROJ_NAME,
                env_variables.git_hash, env_variables.git_tag);

        nob_cc_output(&cmd, bin_name);

        for (size_t i = 0; i < src_files_count; i++) {
            nob_cmd_append(&cmd, src_files[i]);
        }

        if (!nob_cmd_run(&cmd))
            return 1;
    }

    // run
    if (!no_run) {
        Nob_Cmd cmd_run = {0};
        nob_cmd_append(&cmd_run, bin_name);
        for (size_t i = 0; i < prg_args_count; i++) {
            nob_cmd_append(&cmd_run, prg_args[i]);
        }
        if (!nob_cmd_run(&cmd_run))
            return 1;
    }

    return 0;
}

// implementations ------------------------------------------------------------

// helper: embed_static(const char *path)
char *format_path_to_name(const char *filename) {
    size_t len = strlen(filename);
    char *result = calloc(len + 1, sizeof(char));

    size_t i = 0;
    if (isdigit(filename[0])) {
        result[0] = 'n';
        i = 1;
    }
    for (; i < len; i++) {
        if (isspace(filename[i]) || filename[i] == '.') {
            result[i] = '_';
        } else {
            result[i] = tolower(filename[i]);
        }
    }
    return result;
}

void embed_static(const char *path) {
    char dir[256] = {0};

    const char *last_slash = strrchr(path, '/');
    if (last_slash) {
        size_t dir_len = last_slash - path + 1;
        if (dir_len >= sizeof(dir)) {
            return;
        }
        memcpy(dir, path, dir_len);
        dir[dir_len] = '\0';
    }

    const char *filename = nob_path_name(path);
    char *formatted_name = format_path_to_name(filename);

    char output_path[512] = {0};
    snprintf(output_path, sizeof(output_path), "%s%s.h", dir, formatted_name);
    FILE *dest_h = fopen(output_path, "w");

    String_Builder h_content = {0};

    String_Builder sb = {0};
    nob_read_entire_file(path, &sb);

    char tmp[256] = {0};
    snprintf(tmp, 256, "unsigned char static_%s[] = {\n", formatted_name);
    sb_append_cstr(&h_content, tmp);

#define COLUMNS 7

    size_t i = 0;
    while (i < sb.count) {
        for (int j = COLUMNS; j > 0 && i < sb.count; j--) {
            char tmp[256] = {0};
            snprintf(tmp, 256, "0x%02X, ", (unsigned char)sb.items[i++]);
            sb_append_cstr(&h_content, tmp);
        }
        sb_append_cstr(&h_content, "\n");
    }

    sb_append_cstr(&h_content, "\n};\n");

    {
        char tmp[256] = {0};
        snprintf(tmp, 256, "\nunsigned int static_%s_len = %zu;\n", formatted_name,
                sb.count);
        sb_append_cstr(&h_content, tmp);
    }

    fwrite(h_content.items, sizeof(char), h_content.count, dest_h);
    fclose(dest_h);

    free(formatted_name);
}

bool collect_src_files(Nob_Walk_Entry entry) {
    if (entry.type == FILE_REGULAR && strstr(entry.path, ".c")) {
        if (src_files_count + 1 >= MAX_SRC_FILES)
            return false;
        src_files[src_files_count++] = strdup(entry.path);
    }
    return true;
}

bool collect_static_files(Nob_Walk_Entry entry) {
    if (entry.type == FILE_REGULAR && !strstr(entry.path, ".h")) {
        if (static_files_count + 1 >= MAX_SRC_FILES)
            return false;
        static_files[static_files_count++] = strdup(entry.path);
    }
    return true;
}

intern_fn void print_help() {
    printf("./nob [options|command]\n\n");
    printf("* Execute without options to compile and run a debug build.\n");
    printf("* Program arguments are passed after a divider '--'.\n\n");
    for (size_t i = 1; i < sizeof(arguments) / sizeof(arguments[1]); i++) {
        if (arguments[i].str_alt != NULL) {
            char tmp[128] = {0};
            snprintf(tmp, 128, "%s, %s", arguments[i].str, arguments[i].str_alt);
            printf("%-22s %s\n", tmp, arguments[i].descr);
        } else {
            printf("%-22s %s\n", arguments[i].str, arguments[i].descr);
        }
    }
}

void append_env_var(Nob_Cmd *cmd, const char *prefix, const char *value,
        bool c_standard) {
    if (value == NULL)
        return;
    char *tmp = c_standard ? nob_temp_sprintf("%s=%s", prefix, value)
        : nob_temp_sprintf("%s=\"%s\"", prefix, value);
    nob_cmd_append(cmd, tmp);
}

#define get_git_details_helper(env)                                            \
    do {                                                                         \
        if (nob_cmd_run(&cmd, .stdout_path = tmp)) {                               \
            String_Builder sb = {0};                                                 \
            if (nob_read_entire_file(tmp, &sb)) {                                    \
                while (sb.count > 0 && (sb.items[sb.count - 1] == '\n' ||              \
                            sb.items[sb.count - 1] == '\r'))               \
                sb.count--;                                                          \
                sb_append_null(&sb);                                                   \
                (env) = strdup(sb.items);                                              \
            }                                                                        \
            sb_free(sb);                                                             \
        }                                                                          \
    } while (0)

intern_fn void get_git_details(ArgKind build_kind) {
    // tag
    {
        Nob_Cmd cmd = {0};
        nob_cmd_append(&cmd, "git", "describe", "--tags", "--abbrev=0");
        char *tmp;
        if (build_kind == C_RELEASE) {
            tmp = DIR_BUILD_RELEASE "git_tag.tmp";
        } else {
            tmp = DIR_BUILD_DEBUG "git_tag.tmp";
        }
        get_git_details_helper(env_variables.git_tag);
    }

    // hash
    {
        Nob_Cmd cmd = {0};
        nob_cmd_append(&cmd, "git", "rev-parse", "HEAD");
        char *tmp;
        if (build_kind == C_RELEASE) {
            tmp = DIR_BUILD_RELEASE "git_hash.tmp";
        } else {
            tmp = DIR_BUILD_DEBUG "git_hash.tmp";
        }
        get_git_details_helper(env_variables.git_hash);
    }
}

/*
   Copyright © 2026 Simon Danielsson

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files, to deal in the Software
   without restriction, including without limitation the rights to use, copy,
   modify, merge, publish, distribute, sublicense, and/or sell copies of the
   Software, and to permit persons to whom the Software is furnished to do so,
   subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
   */
