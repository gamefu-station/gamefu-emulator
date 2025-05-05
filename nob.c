#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_DEPRECATE 1

#if defined(_MSC_VER) && !defined(__clang__)
#    define nob_cc_flags(...) // TODO: Add MSVC flags.
#else
#    define nob_cc_flags(cmd) nob_cmd_append((cmd), "-std=c23", "-Wall", "-Wextra", \
         "-Wno-unused", "-Wno-gnu-zero-variadic-macro-arguments", "-Wno-trigraphs", \
         "-Wno-unused-parameter", "-Wno-unused-function", "-Wno-unused-variable",   \
         "-Werror", "-ggdb", "-Werror=return-type", "-pedantic", "-pedantic-errors", "-fsanitize=address")
#endif

#if defined(_MSC_VER)
#    if defined(__clang__)
#        define nob_ld(cmd) nob_cmd_append((cmd), "clang")
#    else
#        define nob_ld(cmd) nob_cmd_append((cmd), "link.exe")
#    endif
#else
#    define nob_ld(cmd) nob_cmd_append((cmd), "ld")
#endif

#if defined(_MSC_VER)
#    if defined(__clang__)
#        define nob_ld_flags(cmd) nob_cmd_append((cmd), "-fsanitize=address")
#    else
#        define nob_ld_flags(cmd) nob_cmd_append((cmd), "/nologo")
#    endif
#else
#    define nob_ld_flags(cmd) nob_cmd_append((cmd), "-fsanitize=address")
#endif

#if defined(_MSC_VER)
#    if defined(__clang__)
#        define nob_ld_output(cmd, output_path) nob_cmd_append((cmd), "-o", (output_path))
#    else
#        define nob_ld_output(cmd, output_path) nob_cmd_append((cmd), nob_temp_sprintf("/out:%s", (output_path)))
#    endif
#else
#    define nob_ld_output(cmd, output_path) nob_cmd_append((cmd), "-o", (output_path))
#endif

#define nob_ld_inputs(cmd, ...) nob_cmd_append((cmd), __VA_ARGS__)

#if defined(_WIN32)
#    define nob_exe(exe_path) nob_temp_sprintf("%s.exe", (exe_path))
#else
#    define nob_exe(exe_path) exe_path
#endif

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

static bool build_obj(const char* src, const char* obj, Nob_File_Paths* include_files, Nob_File_Paths* out_obj_files) {
    bool result = true;

    Nob_Cmd cmd = {0};

    nob_da_append(out_obj_files, obj);
    if (nob_file_exists(obj)) {
        int rebuild_status = nob_needs_rebuild1(obj, src);
        if (rebuild_status < 0) nob_return_defer(false);

        if (0 == rebuild_status) {
            rebuild_status = nob_needs_rebuild(obj, include_files->items, include_files->count);
            if (rebuild_status < 0) nob_return_defer(false);
        }

        if (0 == rebuild_status) nob_return_defer(true);
    }

    nob_cc(&cmd);
    nob_cc_output(&cmd, obj);
    nob_cmd_append(&cmd, "-c");
    nob_cc_inputs(&cmd, src);
    nob_cmd_append(&cmd, "-Iinclude");
    nob_cc_flags(&cmd);

    if (!nob_cmd_run_sync(cmd)) {
        nob_return_defer(false);
    }

defer:;
    nob_cmd_free(cmd);
    return result;
}

static bool link_exe(const char* exe, Nob_File_Paths* obj_files) {
    bool result = true;

    Nob_Cmd cmd = {0};

    int rebuild_status = nob_needs_rebuild(exe, obj_files->items, obj_files->count);
    if (rebuild_status < 0) nob_return_defer(false);
    if (0 == rebuild_status) nob_return_defer(true);

    nob_ld(&cmd);
    nob_ld_output(&cmd, exe);
    nob_da_append_many(&cmd, obj_files->items, obj_files->count);
    nob_ld_flags(&cmd);

    if (!nob_cmd_run_sync(cmd)) {
        nob_return_defer(false);
    }

defer:;
    nob_cmd_free(cmd);
    return result;
}

static bool build_gfusx(Nob_File_Paths* include_files) {
    bool result = true;

    Nob_File_Paths obj_files = {0};

    if (!build_obj("src/gfusx.c", ".build/gfusx.o", include_files, &obj_files)) {
        nob_return_defer(false);
    }

    if (!link_exe(nob_exe(".build/gfusx"), &obj_files)) {
        nob_return_defer(false);
    }

defer:;
    nob_da_free(obj_files);
    return result;
}

int main(int argc, char** argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);

    int result = 0;

    Nob_File_Paths all_include_paths = {0};
    Nob_Cmd cmd = {0};

    if (!nob_mkdir_if_not_exists(".build")) {
        nob_return_defer(false);
    }

    if (!nob_read_entire_dir_recursive("include", &all_include_paths)) {
        nob_return_defer(1);
    }

    if (!build_gfusx(&all_include_paths)) {
        nob_return_defer(1);
    }

defer:;
    nob_cmd_free(cmd);
    nob_da_free(all_include_paths);
    return result;
}
