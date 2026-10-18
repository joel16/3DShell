// hbloader — chain-load .3dsx payloads like the Homebrew Menu does.
//
// Two modes (first available wins), mirroring fincs/new-hbmenu:
//   ROSALINA: connect to Luma3DS's "hb:ldr" port, then
//     HBLDR_SetTarget (cmd 0x20002): target path (static buffer)
//     HBLDR_SetArgv   (cmd 0x30002): argv buffer  (static buffer)
//   -> when this process exits, hbldr maps the new 3dsx into us.
//   NINJHAX2: the exploit populates the bootloader function table at
//     0x00100000; the chain runs through __system_retAddr on exit.
//
// argv layout (hbmenu spec): u32 argc, then NUL-terminated strings;
// argv[0] = "sdmc:<path>" (full path WITH the sdmc: prefix).
#include <3ds.h>
#include <cstdio>
#include <cstring>
#include <string>

#include "hbloader.h"
#include "log.h"

static HBLoaderArgBuf argbuf;

extern "C" void (*__system_retAddr)(void);

static Handle hbldr_handle = 0;
static bool ninjhax2 = false;
static bool initialized = false;
static bool exit_requested = false;

/* ninjhax 2.0+ bootloader entry point ("boot file in slot") */
typedef void (*callBootloader_2x_fn)(Handle file, u32 *argbuf, u32 arglength);
#define callBootloader_2x ((callBootloader_2x_fn)0x00100000)

static Handle hb_file_handle = 0;

static void bootloaderJump(void) {
    if (hb_file_handle != 0)
        callBootloader_2x(hb_file_handle, argbuf.buffer, argbuf.length);
}

/* HBLDR IPC (identical to new-hbmenu source/loaders/rosalina.c) */
static Result hbldr_set_target(const char *path) {
    u32 path_len = strlen(path) + 1;
    u32 *cmdbuf = getThreadCommandBuffer();
    cmdbuf[0] = IPC_MakeHeader(2, 0, 2); // 0x20002
    cmdbuf[1] = IPC_Desc_StaticBuffer(path_len, 0);
    cmdbuf[2] = (u32)path;
    Result ret = svcSendSyncRequest(hbldr_handle);
    if (R_SUCCEEDED(ret))
        ret = cmdbuf[1];
    return ret;
}

static Result hbldr_set_argv(const void *buffer, u32 size) {
    u32 *cmdbuf = getThreadCommandBuffer();
    cmdbuf[0] = IPC_MakeHeader(3, 0, 2); // 0x30002
    cmdbuf[1] = IPC_Desc_StaticBuffer(size, 1);
    cmdbuf[2] = (u32)buffer;
    Result ret = svcSendSyncRequest(hbldr_handle);
    if (R_SUCCEEDED(ret))
        ret = cmdbuf[1];
    return ret;
}

void HBLoader::Init(void) {
    if (initialized)
        return;

    initialized = true;

    Result ret = svcConnectToPort(&hbldr_handle, "hb:ldr");
    if (R_SUCCEEDED(ret) && hbldr_handle != 0)
        return;

    hbldr_handle = 0;

    /* ninjhax2 fallback. amInit succeeding is not enough by itself on a
     * modern CFW (AM is a real sysmodule), so additionally require the
     * homebrew environment: a .cia has no stub at 0x00100000 and would
     * crash on exit. */
    if (envIsHomebrew() && R_SUCCEEDED(amInit())) {
        ninjhax2 = true;
        Log::Error("HBLoader: ninjhax2 bootloader detected\n");
        return;
    }

    Log::Error("HBLoader: no chainloader available\n");
}

/* Shell-style arg split (quotes respected), ported from hbmenu 1.x boot.c. */
static bool build_argbuf(const char *argv0, const char *args) {
    argbuf.buffer[0] = 0;

    char *dst = reinterpret_cast<char *>(&argbuf.buffer[1]);
    const char *buf_end = reinterpret_cast<char *>(argbuf.buffer + sizeof(argbuf.buffer) / sizeof(argbuf.buffer[0]));

    auto add_arg = [&](const char *arg) -> bool {
        if (!arg)
            return false;
        size_t len = strlen(arg) + 1;
        if ((dst + len) > buf_end)
            return false;
        memcpy(dst, arg, len);
        dst += len;
        argbuf.buffer[0]++;
        return true;
    };

    if (!add_arg(argv0))
        return false;

    char token[256];
    const char *p = (args != nullptr)? args : "";
    while (*p) {
        while ((*p == ' ') || (*p == '\t'))
            p++;
        if (!(*p))
            break;

        char quote = 0;
        if ((*p == '\"') || (*p == '\''))
            quote = *p++;

        size_t n = 0;
        while (*p && ((quote && *p != quote) || (!quote && *p != ' ' && *p != '\t'))) {
            if (n < (sizeof(token) - 1))
                token[n++] = *p;
            p++;
        }
        if (quote && (*p == quote))
            p++;
        token[n] = '\0';

        if (token[0] && !add_arg(token))
            break;
    }

    argbuf.length = (u32)((uintptr_t)dst - (uintptr_t)argbuf.buffer);
    return true;
}

bool HBLoader::Boot(const std::string &path, const std::string &args) {
    if (path.empty())
        return false;

    if (!initialized)
        HBLoader::Init();

    /* Canonical argv[0] form: "sdmc:<path>". */
    static char argv0[512];
    if (path.compare(0, 5, HBLOADER_PATH_PREFIX) == 0)
        snprintf(argv0, sizeof(argv0), "%s%s", HBLOADER_PATH_PREFIX, path.substr(5).c_str());
    else
        snprintf(argv0, sizeof(argv0), "%s%s", HBLOADER_PATH_PREFIX, path.c_str());

    if (!build_argbuf(argv0, args.c_str()))
        return false;

    if (hbldr_handle != 0) {
        /* Rosalina: strip the "sdmc:" prefix for the target path. */
        static char target[512];
        snprintf(target, sizeof(target), "%s", argv0 + 5); /* keep the leading '/' */

        if (R_FAILED(hbldr_set_target(target)) || R_FAILED(hbldr_set_argv(argbuf.buffer, sizeof(argbuf.buffer))))
            return false;

        exit_requested = true;
        return true;
    }

    if (ninjhax2 && envIsHomebrew()) {
        if (R_FAILED(FSUSER_OpenFileDirectly(&hb_file_handle, ARCHIVE_SDMC, fsMakePath(PATH_EMPTY, ""),
            fsMakePath(PATH_ASCII, argv0), FS_OPEN_READ, 0)) || (hb_file_handle == 0))
            return false;

        __system_retAddr = bootloaderJump;
        exit_requested = true;
        return true;
    }

    return false;
}

bool HBLoader::PendingExit(void) {
    if (exit_requested) {
        exit_requested = false;
        return true;
    }
    return false;
}

bool HBLoader::Available(void) {
    if (!initialized)
        HBLoader::Init();
    return ((hbldr_handle != 0) || ninjhax2);
}
