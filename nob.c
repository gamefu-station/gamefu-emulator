#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_DEPRECATE 1

#ifndef nob_cc_flags
#    if defined(_MSC_VER)
#        define nob_cc_flags(...) // TODO: Add MSVC flags.
#    else
#        define nob_cc_flags(cmd) nob_cmd_append(cmd, "-std=c23", "-Wall", "-Wextra", \
             "-Wno-unused", "-Wno-gnu-zero-variadic-macro-arguments", "-Wno-trigraphs", \
             "-Werror", "-ggdb", "-Werror=return-type", "-pedantic", "-pedantic-errors", "-fsanitize=address")
#    endif
#endif // nob_cc_output

#define NOB_IMPLEMENTATION
#include "nob.h"

#define GFUBASE_IMPLEMENTATION
#include "include/gfubase/gfubase.h"

static bool nob_read_entire_dir_recursive(const char* dir, Nob_File_Paths* paths) {
    bool result = true;

    Nob_File_Paths this_paths = {0};
    if (!nob_read_entire_dir(dir, &this_paths)) {
        nob_return_defer(false);
    }

    for (size_t i = 2; i < this_paths.count; i++) {
        const char* child_name = this_paths.items[i];
        const char* child_path = nob_temp_sprintf("%s/%s", dir, child_name);

        Nob_File_Type child_file_type = nob_get_file_type(child_path);
        if (child_file_type == NOB_FILE_DIRECTORY) {
            nob_read_entire_dir_recursive(child_path, paths);
        } else if (child_name[0] != '.') {
            nob_da_append(paths, child_path);
        }
    }

defer:;
    return result;
}

int main(int argc, char** argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);

    int result = 0;

    Nob_File_Paths all_include_paths = {0};
    Nob_Cmd cmd = {0};

    if (!nob_read_entire_dir_recursive("include", &all_include_paths)) {
        nob_return_defer(1);
    }

defer:;
    nob_cmd_free(cmd);
    nob_da_free(all_include_paths);
    return result;
}
