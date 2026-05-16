#define NOB_STRIP_PREFIX
#define NOB_IMPLEMENTATION
#include "nob.h"

#define BUILD "build/"
#define LINUX BUILD "linux/"
#define WINDOWS BUILD "windows/"
#define SRC "src/"
#define EXAMPLES "examples/"
#define ENGINE SRC"creese-2D-engine/"
#define EXTERNAL SRC"external/"

/* if any of these files get touched creese is rebuilt */
const char *creese_2D_srcs[] = {
    SRC"creese_2D.h",
    ENGINE"creese_2D.c",
    ENGINE"time_keep.c",
    ENGINE"sprite.c",
    ENGINE"audio.c",
    ENGINE"input.c",
    ENGINE"swr.h",
};

const char *examples[] = {
    "example_circle",
    "example_image",
    "example_animation",
    "example_text",
    "example_tuxedo_man",
    "example_audio",
    "example_isometric",
    "example_collision",
    "example_dyn_array",
    "example_robot",
    "example_shmup",
};

/* if any of these files get touched game is rebuilt */
const char *game_dep_srcs[] = {
    SRC"game_example_dependency.c",
};

/* header only modules are precompiled */
struct {
    const char *name;
    const char *impl_define;
    const char *api_define;
    const char *include_dir;
} hdr_modules[] = {
    {
        .name = "la",
        .impl_define = "-DLA_IMPLEMENTATION",
        .api_define = "-DLADEF=extern",
        .include_dir = EXTERNAL,
    },
    {
        .name = "RGFW",
        .impl_define = "-DRGFW_IMPLEMENTATION",
        .include_dir = EXTERNAL,
    },
    {
        .name = "stb_image",
        .impl_define = "-DSTB_IMAGE_IMPLEMENTATION",
        .include_dir = EXTERNAL,
    },
    {
        .name = "stb_truetype",
        .impl_define = "-DSTB_TRUETYPE_IMPLEMENTATION",
        .include_dir = EXTERNAL,
    },
    {
        .name = "miniaudio",
        .impl_define = "-DMINIAUDIO_IMPLEMENTATION",
        .include_dir = EXTERNAL,
    },
    {
        .name = "dr_wav",
        .impl_define = "-DDR_WAV_IMPLEMENTATION",
        .include_dir = EXTERNAL,
    },
    {
        .name = "nob",
        .impl_define = "-DNOB_IMPLEMENTATION",
        .include_dir = "./",
    },
};


bool build_header_only_libraries(Cmd *cmd, const char *target, bool force)
{
    for (size_t i = 0; i < ARRAY_LEN(hdr_modules); i++) {
        const char *hdr = temp_sprintf("%s%s.h", hdr_modules[i].include_dir, hdr_modules[i].name);
        const char *obj = temp_sprintf("%s%s.o", (target == "linux") ? LINUX : WINDOWS, hdr_modules[i].name);
        int res = needs_rebuild1(obj, hdr);
        if (res < 0) {
            nob_log(ERROR, "needs rebuild failed: header %s, obj %s\n", hdr, obj);
            return false;
        } else if (!res && !force) {
            continue; // no rebuild necessary
        } else {
            /* build compiler command */
            cmd_append(cmd, (target == "linux") ? "gcc" : "x86_64-w64-mingw32-gcc");
            cmd_append(cmd, hdr_modules[i].impl_define);
            if (hdr_modules[i].api_define) cmd_append(cmd, hdr_modules[i].api_define);
            cmd_append(cmd, "-x", "c", hdr);
            cmd_append(cmd, "-c", "-o", obj);
            if (!cmd_run(cmd)) return false;
        }
    }
    return true;
}

bool build_creese_2D(Cmd *cmd, bool force, const char *target)
{
    /* build director strings and return early if no rebuild is necessary */
    const char *build_dir = (target == "linux") ? LINUX : WINDOWS;
    const char *obj = temp_sprintf("%screese_2D.o", build_dir);
    int creese_touched = needs_rebuild(obj, creese_2D_srcs, ARRAY_LEN(creese_2D_srcs));
    if (creese_touched < 0) return false;
    if (!creese_touched && !force) return true;

    /* build compiler command */
    cmd_append(cmd, (target == "linux") ? "gcc" : "x86_64-w64-mingw32-gcc");
    if (target == "linux") cmd_append(cmd, "-Wall", "-Wextra", "-Werror", "-g");
    cmd_append(cmd, "-c", ENGINE"creese_2D.c");
    cmd_append(cmd, "-o", obj);
    if (!cmd_run(cmd)) return false;

    return true;
}

bool build_example(Cmd *cmd, const char *example_name, const char *target)
{
    /* build director strings and return early if no rebuild is necessary */
    const char *build_dir = (target == "linux") ? LINUX : WINDOWS;
    const char *src  = temp_sprintf(SRC"%s.c", example_name);
    const char *exec = temp_sprintf("%s%s%s", build_dir, example_name, (target == "windows") ? ".exe" : "");
    const char *creese_obj = temp_sprintf("%screese_2D.o", build_dir);
    int src_touched = needs_rebuild1(exec, src);
    int creese_touched = needs_rebuild1(exec, creese_obj);
    if (src_touched < 0 || creese_touched < 0) {
        nob_log(ERROR, "failed to rebuild for creese or example %s", example_name);
        return false;
    }
    if (!src_touched && !creese_touched) return true;

    /* build compiler command */
    cmd_append(cmd, (target == "linux") ? "gcc" : "x86_64-w64-mingw32-gcc");
    if (target == "linux") cmd_append(cmd, "-Wall", "-Wextra", "-g");
    // cmd_append(cmd, "-Wno-missing-braces");
    for (size_t i = 0; i < ARRAY_LEN(hdr_modules); i++)
        cmd_append(cmd, temp_sprintf("%s%s.o", (target == "linux") ? LINUX : WINDOWS, hdr_modules[i].name));
    cmd_append(cmd, creese_obj);
    cmd_append(cmd, src);
    cmd_append(cmd, "-o", exec);
    cmd_append(cmd, "-lm");
    if (target == "linux") cmd_append(cmd, "-lXrandr", "-lX11");
    else                   cmd_append(cmd, "-lgdi32");
    return cmd_run(cmd);
}

bool build_game(Cmd *cmd, const char *example_name, const char *target)
{
    /* build director strings and return early if no rebuild is necessary */
    const char *build_dir = (target == "linux") ? LINUX : WINDOWS;
    const char *src  = temp_sprintf(SRC"%s.c", example_name);
    const char *exec = temp_sprintf("%s%s%s", build_dir, example_name, (target == "windows") ? ".exe" : "");
    const char *creese_obj = temp_sprintf("%screese_2D.o", build_dir);
    int src_touched = needs_rebuild1(exec, src);
    int creese_touched = needs_rebuild1(exec, creese_obj);
    int game_deps_touched = needs_rebuild(exec, game_dep_srcs, ARRAY_LEN(game_dep_srcs));
    if (src_touched < 0 || creese_touched < 0 || game_deps_touched < 0) {
        nob_log(ERROR, "failed to rebuild creese 2D game (needs rebuild failed for some reason)");
        return false;
    }
    if (!src_touched && !creese_touched && !game_deps_touched) return true;

    /* build compiler command */
    cmd_append(cmd, (target == "linux") ? "gcc" : "x86_64-w64-mingw32-gcc");
    if (target == "linux") cmd_append(cmd, "-Wall", "-Wextra", "-g");
    // cmd_append(cmd, "-Wno-missing-braces");
    for (size_t i = 0; i < ARRAY_LEN(hdr_modules); i++)
        cmd_append(cmd, temp_sprintf("%s%s.o", (target == "linux") ? LINUX : WINDOWS, hdr_modules[i].name));
    cmd_append(cmd, creese_obj);
    cmd_append(cmd, src);
    cmd_append(cmd, "-o", exec);
    cmd_append(cmd, "-lm");
    if (target == "linux") cmd_append(cmd, "-lXrandr", "-lX11");
    else                   cmd_append(cmd, "-lgdi32");
    return cmd_run(cmd);
}

void log_usage(const char *program)
{
    printf("usage: %s [options]\n", program);
    printf("    --help\n");
    printf("    --clean, force clean build\n");
    printf("    --target, build target (e.g. windows and linux)\n");
    printf("    --run, run game after building (linux only)\n");
    printf("    --debug, launch debugger after building (relies on gf2, to install go to\n");
    printf("             https://github.com/nakst/gf and build from source then put gf2 in your path)\n");
    printf("    --examples, for the game jam we only want to build examples if we explicity say so\n");
}

struct {
    const char *program;
    const char *target;
    bool clean;
    bool game_run;
    bool game_debug;
    bool build_examples;
} config = {0};

bool parse_cmd_args(int argc, char **argv)
{
    config.program = shift(argv, argc);

    while (argc) {
        const char *flag = shift(argv, argc);
        if (!strcmp("--help", flag)) {
            log_usage(config.program);
            return false;
        } else if (!strcmp("--clean", flag)) {
            config.clean = true;
            nob_log(INFO, "executing clean build");
        } else if (!strcmp("--run", flag)) {
            config.game_run = true;
        } else if (!strcmp("--debug", flag)) {
            config.game_debug = true;
        } else if (!strcmp("--examples", flag)) {
            config.build_examples = true;
        } else if (!strcmp("--target", flag)) {
            const char *target = shift(argv, argc);
            if (!strcmp(target, "windows")) config.target = "windows";
            if (!strcmp(target, "linux"))   config.target = "linux";
        } else {
            nob_log(ERROR, "unrecognized flag %s", flag);
            log_usage(config.program);
            return false;
        }
    }

    if (config.game_run && config.game_debug) {
        nob_log(ERROR, "`--run` and `--debug` command should be mutually exclusive");
        return false;
    }

    /* default target linux */
    if (!config.target) config.target = "linux";

    return true;
}

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    if (!parse_cmd_args(argc, argv))   return 1;
    if (!mkdir_if_not_exists(BUILD))   return 1;
    if (!mkdir_if_not_exists(LINUX))   return 1;
    if (!mkdir_if_not_exists(WINDOWS)) return 1;

    Cmd cmd = {0};

    if (!build_header_only_libraries(&cmd, config.target, config.clean)) return 1;
    if (!build_creese_2D(&cmd, config.clean, config.target))             return 1;

    /* The below section has been modified for the game jam */

    if (config.build_examples) {
        for (size_t i = 0; i < ARRAY_LEN(examples); i++)
            if (!build_example(&cmd, examples[i], config.target)) return 1;
    }

    if (!build_game(&cmd, "game_main", config.target)) return 1;

    if (config.game_run) {
        cmd_append(&cmd, "./build/linux/game_main");
        cmd_run(&cmd);
    }

    if (config.game_debug) {
        cmd_append(&cmd, "gf2", "-ex", "start", "./build/linux/game_main");
        cmd_run(&cmd);
    }

    return 0;
}
