/*==============================================================
 * src/cnc/cnc_dwg.c
 * DXF -> DWG export by shelling out to an external converter.
 *
 * We do NOT hand-write DWG bytes: it's a closed, versioned binary
 * format with no public spec, unlike DXF. This wraps whatever DXF
 * we already write correctly and converts it, rather than pretending
 * to natively emit DWG.
 *
 * Converter: GNU LibreDWG's dxf2dwg. Unlike ODA File Converter, this
 * builds and runs natively on ARM, so it works directly on the Pi --
 * no x86_64 host/post-process step needed.
 *==============================================================*/

#include "cnc_dwg.h"
#include "cnc_dxf.h"
#include "cnc_sketch.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

#ifndef CNC_DWG_CONVERTER_BIN
#define CNC_DWG_CONVERTER_BIN "dxf2dwg"
#endif

/* dxf2dwg signature (LibreDWG):
 *   dxf2dwg [OPTION]... DXFFILES ...
 *     -y                 overwrite existing output without asking
 *     --as rNNNN         save-as version (r12, r14, r2000 default, r2004)
 *     -o outfile         explicit output path (only valid with ONE input DXF)
 *
 * We always call it with exactly one DXF and an explicit -o path, so
 * -o's single-file restriction is never an issue.
 *
 * NOTE: every printf() here is followed by fflush(stdout). This app's
 * stdout is not a TTY (framebuffer GUI, redirected/journald), so glibc
 * fully block-buffers it by default -- without the flush, these
 * diagnostics can sit in the buffer indefinitely and never show up in
 * the log, making a real failure look like total silence.
 */
static bool run_dxf_to_dwg_converter(const char *dxf_path, const char *dwg_path)
{
    pid_t pid = fork();
    if (pid < 0) {
        perror("[cnc_dwg] fork failed");
        return false;
    }

    if (pid == 0) {
        /* Child: exec the converter directly (no shell -> no injection risk
         * from filenames). Redirect its noisy stdout so it doesn't spam
         * the console; keep stderr so real failures are still visible. */
        freopen("/dev/null", "w", stdout);
        execlp(CNC_DWG_CONVERTER_BIN, CNC_DWG_CONVERTER_BIN,
               "-y", "--as", "r2000", "-o", dwg_path, dxf_path,
               (char *)NULL);
        /* Only reached if execlp itself failed (binary missing/not on PATH) */
        _exit(127);
    }

    int status = 0;
    if (waitpid(pid, &status, 0) < 0) {
        perror("[cnc_dwg] waitpid failed");
        return false;
    }

    if (!WIFEXITED(status)) {
        printf("[cnc_dwg] converter process terminated abnormally (signal)\n");
        fflush(stdout);
        return false;
    }

    int rc = WEXITSTATUS(status);
    if (rc == 127) {
        printf("[cnc_dwg] '%s' not found on PATH.\n", CNC_DWG_CONVERTER_BIN);
        fflush(stdout);
        return false;
    }
    if (rc != 0) {
        printf("[cnc_dwg] converter exited with code %d\n", rc);
        fflush(stdout);
    }
    return rc == 0;
}

bool cnc_sketch_save_dwg(const char *filename)
{
    /* filename is expected WITHOUT extension, e.g. "sketch_20260807_140212" */
    char dxf_name[80];
    snprintf(dxf_name, sizeof(dxf_name), "%s.dxf", filename);

    if (!cnc_sketch_save_dxf(dxf_name)) {
        printf("[cnc_dwg] Source DXF write failed -- aborting DWG export.\n");
        fflush(stdout);
        return false;
    }

    char dxf_path[256];
    cnc_get_sketch_path(dxf_name, dxf_path, sizeof(dxf_path));

    struct stat st;
    if (stat(dxf_path, &st) != 0) {
        printf("[cnc_dwg] Expected DXF at %s but it's missing.\n", dxf_path);
        fflush(stdout);
        return false;
    }

    char dwg_name[80];
    snprintf(dwg_name, sizeof(dwg_name), "%s.dwg", filename);
    char dwg_path[256];
    cnc_get_sketch_path(dwg_name, dwg_path, sizeof(dwg_path));

    if (!run_dxf_to_dwg_converter(dxf_path, dwg_path)) {
        printf("[cnc_dwg] DWG conversion failed via '%s'.\n", CNC_DWG_CONVERTER_BIN);
        fflush(stdout);
        return false;
    }

    printf("[cnc_dwg] DWG saved: %s\n", dwg_path);
    fflush(stdout);
    return true;
}