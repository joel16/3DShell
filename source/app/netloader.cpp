// netloader — 3dslink receiver (hbmenu-compatible protocol).
//
// Wire protocol (devkitPro/3dslink, confirmed against hbmenu 1.x
// source/netloader.c and the 3dslink host implementation):
//   1. UDP 17491: host broadcasts "3dsboot" (source port = its recv
//      socket, also 17491); device answers "boot3ds" on port 17491.
//   2. TCP 17491: one connection per transfer. Host sends:
//        s32 namelen, <name>, s32 filelen
//      Device opens sdmc:/3ds/<name> (WRITE|CREATE), pre-reserves
//      filelen via SetSize and answers s32: 0 ok / -1 create fail /
//      -2 size fail.
//   3. Host streams zlib chunks: s32 csize + <csize bytes> until the
//      deflate stream ends. Device inflates and appends to the file.
//   4. Device answers s32 response again, then the host sends
//      s32 cmdlen + <cmdlen bytes> of packed argv (argv[0] =
//      "sdmc:/3ds/<name>", extra args NUL-terminated). No argc is
//      sent; strings are scanned NUL to NUL.
#include <3ds.h>
#include <arpa/inet.h>
#include <cerrno>
#include <codecvt>
#include <fcntl.h>
#include <locale>
#include <malloc.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <zlib.h>

#include "fs.h"
#include "gui.h"
#include "hbloader.h"
#include "log.h"
#include "net.h"
#include "netloader.h"

#define NL_RECV_CHUNK (16 * 1024) /* 3dslink ZLIB_CHUNK upper bound */
#define NL_OUT_CHUNK (256 * 1024)
#define NL_WRITE_CHUNK (512 * 1024)

static void DisplayProgress(const std::string &message, s64 offset, s64 size, s64 fallback_size = 1) {
    s64 total = (size < 1)? fallback_size : size;
    total = (total < offset)? offset : total;
    GUI::ProgressBar("NetLoader", message, offset, total);
}

/* Read exactly size bytes from a blocking socket. */
static bool NL_Reply(int sock, s32 response) {
    return (send(sock, &response, sizeof(response), 0) == (ssize_t)sizeof(response));
}

static bool NL_WaitAll(int sock, void *buffer, size_t size) {
    u8 *ptr = static_cast<u8 *>(buffer);
    while (size > 0) {
        ssize_t n = recv(sock, ptr, size, 0);
        if (n < 0 && ((errno == EAGAIN) || (errno == EWOULDBLOCK)))
            continue;

        if (n <= 0)
            return false;

        ptr += n;
        size -= n;
    }
    return true;
}

/* Accumulate inflated output and commit to SD in large batches: an
 * explicit FS_WRITE_FLUSH per chunk (like hbmenu's stdio buffering does
 * implicitly with a 64KB setvbuf) costs one FAT commit transaction each
 * and dominates transfer time. hbmenu parity: flush once at the end. */
static bool NL_WriteBuffered(Handle file, u8 *wrbuf, size_t *wrpos, const u8 *data, size_t have, u64 *offset) {
    if (*wrpos + have > NL_WRITE_CHUNK) {
        u32 written = 0;
        if (R_FAILED(FSFILE_Write(file, &written, *offset, wrbuf, *wrpos, FS_WRITE_UPDATE_TIME)))
            return false;
        *offset += *wrpos;
        *wrpos = 0;
    }

    memcpy(wrbuf + *wrpos, data, have);
    *wrpos += have;
    return true;
}

/* Inflate the chunk stream into an open FSFILE. */
static bool NL_Transfer(int sock, Handle file, const char *name, u32 filelen, size_t *out_total) {
    int ret = 0;
    z_stream strm;
    memset(&strm, 0, sizeof(strm));

    u8 *inbuf = static_cast<u8 *>(malloc(NL_RECV_CHUNK));
    u8 *outbuf = static_cast<u8 *>(malloc(NL_OUT_CHUNK));
    u8 *wrbuf = static_cast<u8 *>(malloc(NL_WRITE_CHUNK));
    size_t wrpos = 0;

    if (!inbuf || !outbuf || !wrbuf) {
        free(inbuf);
        free(outbuf);
        free(wrbuf);
        return false;
    }

    if (R_FAILED(ret = inflateInit(&strm))) {
        free(inbuf);
        free(outbuf);
        free(wrbuf);
        return false;
    }

    size_t total = 0;
    u64 offset = 0;
    bool ok = true;
    size_t last_redraw = 0;
    u64 last_redraw_time = osGetTime();

    while (ok && (ret != Z_STREAM_END)) {
        s32 csize = 0;
        if (!NL_WaitAll(sock, &csize, sizeof(csize)) || (csize < 0) || ((u32)csize > NL_RECV_CHUNK)) {
            ok = false;
            break;
        }

        if (!NL_WaitAll(sock, inbuf, (size_t)csize)) {
            ok = false;
            break;
        }

        strm.avail_in = (u32)csize;
        strm.next_in = inbuf;

        do {
            strm.avail_out = NL_OUT_CHUNK;
            strm.next_out = outbuf;

            if (R_FAILED(ret = inflate(&strm, Z_NO_FLUSH)) || (ret == Z_DATA_ERROR) || (ret == Z_MEM_ERROR) || (ret == Z_STREAM_ERROR)) {
                ok = false;
                break;
            }

            u32 have = (NL_OUT_CHUNK - strm.avail_out);
            if (have > 0) {
                if (!NL_WriteBuffered(file, wrbuf, &wrpos, outbuf, have, &offset)) {
                    ok = false;
                    break;
                }
                total += have;
            }
        } while (strm.avail_out == 0);

        if (ok && (total > filelen)) { /* corrupt stream, over the declared size */
            ok = false;
            break;
        }

        if (ok && ((total - last_redraw > (256 * 1024)) || (ret == Z_STREAM_END))) {
            u64 now = osGetTime();
            u64 elapsed = (now > last_redraw_time)? (now - last_redraw_time) : 1;
            u32 speed_kb = (u32)((total - last_redraw) * 1000 / elapsed / 1024);

            char speed_msg[64];
            char short_name[19] = {0};
            size_t name_len = strlen(name);
            if (name_len > 17)
                snprintf(short_name, sizeof(short_name), "%.15s..", name);
            else
                snprintf(short_name, sizeof(short_name), "%s", name);
            snprintf(speed_msg, sizeof(speed_msg), "%s - %u KB/s", short_name, speed_kb);
            last_redraw = total;
            last_redraw_time = now;
            DisplayProgress(speed_msg, total, filelen);
        }
    }

    /* Commit whatever is left in the write buffer, then flush to SD once. */
    if (ok) {
        if (wrpos > 0) {
            u32 written = 0;
            if (R_FAILED(FSFILE_Write(file, &written, offset, wrbuf, wrpos, FS_WRITE_UPDATE_TIME)))
                ok = false;
            offset += wrpos;
        }

        if (ok)
            ok = R_SUCCEEDED(FSFILE_Flush(file));
    }

    if (ok && ret == Z_STREAM_END)
        ret = Z_OK;
    else if (ok)
        ret = Z_DATA_ERROR;

    inflateEnd(&strm);
    free(inbuf);
    free(outbuf);
    free(wrbuf);
    *out_total = total;
    return (ret == Z_OK);
}

static int NL_RunTransfer(int datafd) {
    s32 namelen = 0;
    s32 filelen = 0;
    s32 response = -1;
    char name[256] = {};
    Handle file = 0;

    if (!NL_WaitAll(datafd, &namelen, sizeof(namelen)) || (namelen <= 0) || (namelen >= (s32)sizeof(name))) {
        Log::Error("NetLoader: invalid name length\n");
        close(datafd);
        return NL_ERROR;
    }

    if (!NL_WaitAll(datafd, name, (size_t)namelen) || !NL_WaitAll(datafd, &filelen, sizeof(filelen))) {
        close(datafd);
        return NL_ERROR;
    }
    name[namelen] = '\0';

    std::string path = "sdmc:/3ds/";
    path.append(name);
    Log::Error("NetLoader: header name=%s len=%d\n", name, filelen);

    /* Ensure /3ds exists, then open sdmc:/3ds/<name> (WRITE|CREATE). */
    std::u16string dir_u16 = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> {}.from_bytes("/3ds");
    (void)FSUSER_CreateDirectory(sdmc_archive, fsMakePath(PATH_UTF16, dir_u16.c_str()), 0);

    std::u16string path_u16 = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> {}.from_bytes(path.substr(5));

    if (R_SUCCEEDED(FSUSER_OpenFile(&file, sdmc_archive, fsMakePath(PATH_UTF16, path_u16.c_str()), FS_OPEN_WRITE | FS_OPEN_CREATE, 0))) {
        response = R_SUCCEEDED(FSFILE_SetSize(file, (u64)filelen))? 0 : -2;
        if (R_FAILED(response))
            FSFILE_Close(file);
    }

    NL_Reply(datafd, response);
    if (response != 0) {
        Log::Error("NetLoader: file open/presize failed (%d)\n", response);
        close(datafd);
        return NL_ERROR;
    }

    DisplayProgress(name, 0, filelen);

    size_t total = 0;
    if (!NL_Transfer(datafd, file, name, filelen, &total)) {
        Log::Error("NetLoader: transfer failed after %zu bytes\n", total);
        FSFILE_Close(file);
        close(datafd);
        return NL_ERROR;
    }
    FSFILE_Close(file);

    NL_Reply(datafd, 0);
    Log::Error("NetLoader: transfer ok: %zu bytes\n", total);

    /* 4. command line (packed argv; argv[0] first). No argc is sent. */
    s32 cmdlen = 0;
    static char cmds[0x300];
    std::string args;

    if (NL_WaitAll(datafd, &cmdlen, sizeof(cmdlen)) && (cmdlen > 0) && (cmdlen <= (s32)sizeof(cmds))) {
        if (NL_WaitAll(datafd, cmds, (size_t)cmdlen)) {
            char *ptr = cmds;
            char *end = cmds + cmdlen;
            bool first = true;

            while ((ptr < end) && (*ptr)) {
                if (!first)
                    args.append(" ");
                args.append(ptr);
                first = false;
                ptr += strlen(ptr) + 1;
            }
        }
    }

    close(datafd);

    /* 5. chain-load the received 3dsx (hbldr rebuilds argv[0] from path). */
    if (HBLoader::Boot(path, args)) {
        Log::Error("NetLoader: launching %s\n", path.c_str());
        return NL_RECEIVED;
    }

    return NL_NOCHAIN;
}

NetLoaderResult NetLoader::Run(void) {
    int udpfd = -1, listenfd = -1;
    NetLoaderResult result = NL_CANCELLED;
    struct sockaddr_in addr;
    struct sockaddr_in udp_remote;
    char recvbuf[64] = {};
    socklen_t remotelen = sizeof(udp_remote);

    if (R_FAILED(Net::Init())) {
        DisplayProgress("Network init failed", 0, 1);
        return NL_ERROR;
    }

    /* Wait for a Wi-Fi connection before opening sockets. Cancelable. */
    while (!Net::GetNetworkStatus()) {
        DisplayProgress("Waiting for connection...", 0, 1);
        hidScanInput();
        svcSleepThread(100000000ULL); /* 100ms between AC probes */
        if (hidKeysDown() & (KEY_B | KEY_START))
            return NL_CANCELLED;
        if (!aptMainLoop())
            return NL_CANCELLED;
    }

    /* UDP discovery socket (non-blocking). */
    udpfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpfd < 0) {
        DisplayProgress("UDP socket failed", 0, 1);
        return NL_ERROR;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(NETLOADER_PORT);

    if (bind(udpfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        goto nl_fail;

    {
        int flags = fcntl(udpfd, F_GETFL, 0);
        fcntl(udpfd, F_SETFL, flags | O_NONBLOCK);
    }

    /* TCP data socket. */
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0)
        goto nl_fail;

    if (bind(listenfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        goto nl_fail;

    {
        int flags = fcntl(listenfd, F_GETFL, 0);
        fcntl(listenfd, F_SETFL, flags | O_NONBLOCK);
    }

    if (listen(listenfd, 5) < 0)
        goto nl_fail;

    struct in_addr self_addr;
    self_addr.s_addr = gethostid();
    Log::Error("NetLoader: listening on %s:%d\n", inet_ntoa(self_addr), NETLOADER_PORT);
    char listen_msg[64];
    snprintf(listen_msg, sizeof(listen_msg), "3dslink to %s:%d", inet_ntoa(self_addr), NETLOADER_PORT);

    while (aptMainLoop()) {
        hidScanInput();

        /* UDP discovery: "3dsboot" -> reply "boot3ds" (port 17491). */
        ssize_t len = recvfrom(udpfd, recvbuf, sizeof(recvbuf), 0, (struct sockaddr *)&udp_remote, &remotelen);
        if ((len > 0) && (strncmp(recvbuf, "3dsboot", strlen("3dsboot")) == 0)) {
            udp_remote.sin_port = htons(NETLOADER_PORT);
            (void)sendto(udpfd, "boot3ds", strlen("boot3ds"), 0, (struct sockaddr *)&udp_remote, sizeof(udp_remote));
        }

        int datafd = accept(listenfd, nullptr, nullptr);
        if (datafd >= 0) {
            /* Accepted sockets inherit O_NONBLOCK on 3DS: clear it so the
             * transfer path blocks on recv instead of spinning on EAGAIN. */
            int flags = fcntl(datafd, F_GETFL, 0);
            fcntl(datafd, F_SETFL, flags & ~O_NONBLOCK);

            close(listenfd);
            listenfd = -1;

            result = static_cast<NetLoaderResult>(NL_RunTransfer(datafd));
            break;
        }

        DisplayProgress(listen_msg, 0, 1);
        svcSleepThread(16000000ULL);

        u32 kDown = hidKeysDown();
        if (kDown & (KEY_B | KEY_START))
            break;
    }

nl_fail:
    if (udpfd >= 0)
        close(udpfd);
    if (listenfd >= 0)
        close(listenfd);

    return result;
}
