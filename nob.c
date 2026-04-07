#define NOB_STRIP_PREFIX
#define NOB_IMPLEMENTATION
#include "nob.h"

#define BUILD "build/"
#define LINUX BUILD "linux/"
#define SRC "src/"
#define EXAMPLES "examples/"
#define ENGINE SRC"creese-2D-engine/"
#define EXTERNAL SRC"external/"

/* if any of these files get touched creese is rebuilt */
const char *creese_2D_srcs[] = {
    SRC"creese_2D.h",
    ENGINE"creese_2D.c",
    ENGINE"time_keep.c",
    ENGINE"swr.h",
};

bool build_header_only_libraries(Cmd *cmd, bool force)
{
    int res = 0;
    res = needs_rebuild1(LINUX"la.o", EXTERNAL"la.h");
    if (res < 0) return false;
    res = needs_rebuild1(LINUX"RGFW.o", EXTERNAL"RGFW.h");
    if (res < 0) return false;
    res = needs_rebuild1(LINUX"stb_image.o", EXTERNAL"stb_image.h");
    if (res < 0) return false;
    res = needs_rebuild1(LINUX"stb_truetype.o", EXTERNAL"stb_truetype.h");
    if (res < 0) return false;
    if (!res &&!force) return true;

    cmd_append(cmd, "gcc", "-g");
    cmd_append(cmd, "-DLA_IMPLEMENTATION", "-DLADEF=static");
    cmd_append(cmd, "-x", "c", EXTERNAL"la.h");
    cmd_append(cmd, "-c", "-o", LINUX"la.o");
    if (!cmd_run(cmd)) return false;

    cmd_append(cmd, "gcc", "-g");
    cmd_append(cmd, "-DRGFW_IMPLEMENTATION");
    cmd_append(cmd, "-x", "c", EXTERNAL"RGFW.h");
    cmd_append(cmd, "-c", "-o", LINUX"RGFW.o");
    if (!cmd_run(cmd)) return false;

    cmd_append(cmd, "gcc", "-g");
    cmd_append(cmd, "-DSTB_IMAGE_IMPLEMENTATION");
    cmd_append(cmd, "-x", "c", EXTERNAL"stb_image.h");
    cmd_append(cmd, "-c", "-o", LINUX"stb_image.o");
    if (!cmd_run(cmd)) return false;

    cmd_append(cmd, "gcc", "-g");
    cmd_append(cmd, "-DSTB_TRUETYPE_IMPLEMENTATION");
    cmd_append(cmd, "-x", "c", EXTERNAL"stb_truetype.h");
    cmd_append(cmd, "-c", "-o", LINUX"stb_truetype.o");
    if (!cmd_run(cmd)) return false;

    cmd_append(cmd, "gcc", "-g");
    cmd_append(cmd, "-DNOB_IMPLEMENTATION");
    cmd_append(cmd, "-x", "c", "nob.h");
    cmd_append(cmd, "-c", "-o", LINUX"nob.o");
    if (!cmd_run(cmd)) return false;

    return true;
}

bool build_creese_2D_linux(Cmd *cmd, bool force)
{
    int creese_touched = needs_rebuild(LINUX"creese_2D.o", creese_2D_srcs, ARRAY_LEN(creese_2D_srcs));
    if (creese_touched < 0) return false;
    if (!creese_touched && !force) return true;

    cmd_append(cmd, "gcc", "-Wall", "-Wextra", "-Werror", "-g");
    cmd_append(cmd, "-c", ENGINE"creese_2D.c");
    cmd_append(cmd, "-o", LINUX"creese_2D.o");
    if (!cmd_run(cmd)) return false;

    return true;
}

bool build_example_linux(Cmd *cmd, const char *example_name)
{
    const char *src  = temp_sprintf(SRC"%s.c", example_name);
    const char *exec = temp_sprintf(LINUX"%s", example_name);
    int src_touched = needs_rebuild1(exec, src);
    int creese_touched = needs_rebuild1(exec, LINUX"creese_2D.o");
    if (src_touched < 0 || creese_touched < 0) return false;
    if (!src_touched && !creese_touched) return true;


    cmd_append(cmd, "gcc", "-Wall", "-Wextra", "-Werror", "-g");
    cmd_append(cmd, LINUX"nob.o");
    cmd_append(cmd, LINUX"la.o");
    cmd_append(cmd, LINUX"RGFW.o");
    cmd_append(cmd, LINUX"stb_image.o");
    cmd_append(cmd, LINUX"stb_truetype.o");
    cmd_append(cmd, LINUX"creese_2D.o");
    cmd_append(cmd, src);
    cmd_append(cmd, "-o", exec);
    cmd_append(cmd, "-lm", "-lXrandr", "-lX11");
    return cmd_run(cmd);
}

void log_usage(const char *program)
{
    printf("usage: %s [options]\n", program);
    printf("    --help\n");
    printf("    --clean, force clean build\n");
}

struct {
    const char *program;
    bool clean;
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
        } else {
            nob_log(ERROR, "unrecognized flag %s", flag);
            log_usage(config.program);
            return false;
        }
    }

    return true;
}

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    if (!parse_cmd_args(argc, argv)) return 1;
    if (!mkdir_if_not_exists(BUILD)) return 1;
    if (!mkdir_if_not_exists(LINUX)) return 1;

    Cmd cmd = {0};

    if (!build_header_only_libraries(&cmd, config.clean)) return 1;
    if (!build_creese_2D_linux(&cmd, config.clean))       return 1;
    if (!build_example_linux(&cmd, "example_circle"))     return 1;
    if (!build_example_linux(&cmd, "example_image"))      return 1;
    if (!build_example_linux(&cmd, "example_animation"))  return 1;
    if (!build_example_linux(&cmd, "example_text"))       return 1;

    return 0;
}
