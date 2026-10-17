// smdh — extract title metadata + 48x48 icon from SMDH data, with an
// LRU texture cache for list rendering.
//
// Source of truth: hbmenu 1.x (source/smdh.c + source/filesystem.c).
// hbmenu decodes every icon eagerly at directory scan time into an
// inline RGB24 buffer drawn with software sprites. We render through
// citro2d, so each icon becomes a GPU texture; to keep frame time
// stable the decode is budgeted (one per Pump call, made from the
// control phase, never inside a C3D frame) and results are kept in a
// small LRU cache keyed by the full UTF16 path.
//
// Sourcing order for a .3dsx path (hbmenu filesystem.c):
//   1. SMDH embedded in the 3dsx (3DSX header carries smdhOffset when
//      headerSize >= 44).
//   2. Sidecar file with same name + ".smdh".
//   3. Sidecar file with same name + ".icn".
// Icon data is bigIconData[0x900] (RGB565, 48x48) stored *tiled* with
// 8x8 Morton blocks (hbmenu tileOrder, from bch2obj.py), opaque. We
// detile to a linear RGBA8 64x64 buffer (icon top-left, transparent),
// then upload through the GSP display transfer with OUT_TILED(1),
// which produces the 3x2-tiled layout PICA expects (the runtime
// equivalent of what tex3ds bakes at build time).

#include <3ds.h>
#include <algorithm>
#include <codecvt>
#include <cstdio>
#include <cstring>
#include <locale>
#include <vector>

#include "fs.h"
#include "log.h"
#include "smdh.h"

#define SMDH_MAGIC_3DSX 0x58534433 // '3DSX'
#define SMDH_MAGIC 0x48444D53 // 'SMDH'
#define SMDH_CACHE_MAX 12
#define SMDH_FAIL_RETRY_FRAMES 180 /* ~3s at 60fps before retrying */

typedef struct SmdhHeader {
    u32 magic;
    u16 version;
    u16 reserved;
} SmdhHeader;

typedef struct SmdhTitle {
    u16 shortDescription[0x40];
    u16 longDescription[0x80];
    u16 publisher[0x40];
} SmdhTitle;

typedef struct SmdhSettings {
    u8 gameRatings[0x10];
    u32 regionLock;
    u8 matchMakerId[0xC];
    u32 flags;
    u16 eulaVersion;
    u16 reserved;
    u32 defaultFrame;
    u32 cecId;
} SmdhSettings;

typedef struct Smdh {
    SmdhHeader header;
    SmdhTitle applicationTitles[16];
    SmdhSettings settings;
    u8 reserved[0x8];
    u8 smallIconData[0x480];
    u16 bigIconData[0x900];
} Smdh;

/* 3DSX header (hbmenu _3DSX_Header): ends after fsOffset = 40 bytes. */
typedef struct Smdh3dsxHeader {
    u32 magic;
    u16 headerSize, relocHdrSize;
    u32 formatVer;
    u32 flags;
    u32 codeSegSize, rodataSegSize, dataSegSize, bssSize;
    u32 smdhOffset, smdhSize;
    u32 fsOffset;
} Smdh3dsxHeader;

/* hbmenu 1.x tileOrder: 8x8 Morton tile offsets, from bch2obj.py.
 * bigIconData stores each 8x8 block with these intra-block offsets. */
static const u8 hb_tile_order[64] = {
    0, 1, 8, 9, 2, 3, 10, 11, 16, 17, 24, 25, 18, 19, 26, 27,
    4, 5, 12, 13, 6, 7, 14, 15, 20, 21, 28, 29, 22, 23, 30, 31,
    32, 33, 40, 41, 34, 35, 42, 43, 48, 49, 56, 57, 50, 51, 58, 59,
    36, 37, 44, 45, 38, 39, 46, 47, 52, 53, 60, 61, 54, 55, 62, 63
};

/* tex3ds "3x2" intra-tile offsets (verified byte-for-byte against the
 * official tex3ds tool: Z-interleave of 3 tx/ty bit pairs). */
static u8 tex3ds_tile[64];

static void smdh_build_tables(void) {
    for (int ty = 0; ty < 8; ty++) {
        for (int tx = 0; tx < 8; tx++) {
            tex3ds_tile[(ty << 3) | tx] = (u8)(
                ((tx & 1) << 0) | ((ty & 1) << 1) |
                ((tx & 2) << 1) | ((ty & 2) << 2) |
                ((tx & 4) << 2) | ((ty & 4) << 3));
        }
    }
}

/* Fill tex->data with the 48x48 icon converted to the exact PICA texture
 * layout (tex3ds "3x2") and ABGR texel bytes ([A,B,G,R]). */
static void smdh_fill_tex(C3D_Tex *tex, const Smdh *smdh) {
    u8 *dst = static_cast<u8 *>(tex->data);
    const u16 *data = smdh->bigIconData;

    for (int j = 0; j < SMDH_ICON_HEIGHT; j += 8) {       /* src block row */
        for (int i = 0; i < SMDH_ICON_WIDTH; i += 8) {   /* src block col */
            for (int k = 0; k < 8 * 8; k++) {
                u8 ix = (u8)(i + (hb_tile_order[k] & 0x7));
                u8 iy = (u8)(j + (hb_tile_order[k] >> 3));
                u16 v = *data++;

                u32 unit = ((iy >> 3) * 512 + (ix >> 3) * 64 + tex3ds_tile[((iy & 7) << 3) | (ix & 7)]) * 4;

                dst[unit + 0] = 0xFF;                    /* A */
                dst[unit + 1] = (u8)(((v & 0x1F) << 3) | ((v & 0x1F) >> 2));   /* B */
                dst[unit + 2] = (u8)(((v >> 5) & 0x3F) << 2 | ((v >> 5) & 0x3F) >> 4); /* G */
                dst[unit + 3] = (u8)(((v >> 11) & 0x1F) << 3 | ((v >> 11) & 0x1F) >> 2); /* R */
            }
        }
    }
}

typedef struct SmdhNode {
    std::u16string path;
    SMDHMeta meta;
    u32 last_use;
    u32 fail_stamp; /* frame index when decoding failed, else 0 */
} SmdhNode;

/* UTF16 (fixed-length field terminated or not) -> length-capped u16string. */
static std::u16string smdh_title_u16(const u16 *src, std::size_t max_chars) {
    std::size_t len = 0;
    while ((len < max_chars) && src[len])
        len++;
    return std::u16string(src, src + len);
}

typedef struct SmdhGarbage {
    C3D_Tex *tex;
    const Tex3DS_SubTexture *subtex;
} SmdhGarbage;

static std::vector<SmdhNode> smdh_nodes;
static std::vector<SmdhGarbage> smdh_garbage;
static u32 smdh_frame = 0;

static void smdh_free_garbage(void) {
    /* Free one sweep late: entries garbage-collected during control
     * frame N were last drawn in frame N-1; frame N has been fully
     * synced by C3D_FrameBegin by the time Pump runs. */
    for (SmdhGarbage &g : smdh_garbage) {
        if (g.subtex)
            delete g.subtex;
        if (g.tex) {
            C3D_TexDelete(g.tex);
            delete g.tex;
        }
    }
    smdh_garbage.clear();
}

static void smdh_retire(SmdhNode &node) {
    if (node.meta.has_icon) {
        SmdhGarbage g;
        g.tex = reinterpret_cast<C3D_Tex *>(node.meta.icon.tex);
        g.subtex = node.meta.icon.subtex;
        smdh_garbage.push_back(g);
        node.meta.icon.tex = nullptr;
        node.meta.icon.subtex = nullptr;
        node.meta.has_icon = false;
    }
}

static void smdh_evict_lru(void) {
    while (smdh_nodes.size() > SMDH_CACHE_MAX) {
        /* Evict the least-recently-used entry (biggest age). */
        std::size_t oldest = 0;
        for (std::size_t i = 1; i < smdh_nodes.size(); i++) {
            if (smdh_nodes[i].last_use < smdh_nodes[oldest].last_use)
                oldest = i;
        }
        smdh_retire(smdh_nodes[oldest]);
        smdh_nodes.erase(smdh_nodes.begin() + oldest);
    }
}

static bool smdh_read(Handle file, void *dst, u64 offset, u32 size) {
    u32 bytes_read = 0;
    return R_SUCCEEDED(FSFILE_Read(file, &bytes_read, offset, dst, size)) && (bytes_read == size);
}

/* Load SMDH bytes for a .3dsx (embedded) or its sidecar .smdh/.icn. */
static bool smdh_fetch(const std::u16string &path, Smdh *smdh) {
    size_t ext = path.length() - 5;
    bool is_3dsx = (path.compare(ext, 5, u".3dsx") == 0);
    Handle file = 0;

    if (!is_3dsx)
        return false;

    if (R_FAILED(FSUSER_OpenFile(&file, sdmc_archive, fsMakePath(PATH_UTF16, path.c_str()), FS_OPEN_READ, 0)))
        return false;

    Smdh3dsxHeader hdr;
    bool got = false;

    if (smdh_read(file, &hdr, 0, sizeof(hdr)) && (hdr.magic == SMDH_MAGIC_3DSX) && (hdr.headerSize >= 44))
        got = smdh_read(file, smdh, hdr.smdhOffset, sizeof(Smdh));

    FSFILE_Close(file);
    if (got)
        return true;

    /* Sidecar fallback: name.smdh / name.icn (hbmenu filesystem.c). */
    for (const char16_t *ext16 : {u".smdh", u".icn"}) {
        std::u16string side = path.substr(0, ext).append(ext16);

        if (R_SUCCEEDED(FSUSER_OpenFile(&file, sdmc_archive, fsMakePath(PATH_UTF16, side.c_str()), FS_OPEN_READ, 0))) {
            got = smdh_read(file, smdh, 0, sizeof(Smdh));
            FSFILE_Close(file);
            if (got)
                return true;
        }
    }

    return false;
}

/* Allocate the texture and fill its data with the icon converted to the
 * exact PICA texture layout (verified against the official tex3ds tool)
 * — no GX transfer involved, plain CPU writes into tex->data. */
static bool smdh_make_icon(C2D_Image *out, const Smdh *smdh) {
    C3D_Tex *tex = new C3D_Tex;
    if (!tex)
        return false;

    if (!C3D_TexInit(tex, SMDH_TEX_DIM, SMDH_TEX_DIM, GPU_RGBA8)) {
        delete tex;
        return false;
    }

    smdh_build_tables();
    smdh_fill_tex(tex, smdh);

    C3D_TexSetFilter(tex, GPU_LINEAR, GPU_LINEAR);

    Tex3DS_SubTexture *subtex = new Tex3DS_SubTexture;
    subtex->width = SMDH_ICON_WIDTH;
    subtex->height = SMDH_ICON_HEIGHT;
    /* tex3ds coordinate convention (verified from tex3ds source): PICA V
     * origin is bottom-left, so v = 1 - y/H. top > bottom means "not
     * rotated"; top < bottom makes citro2d apply a 90-degree UV swap. */
    subtex->left = 0.0f;
    subtex->top = 1.0f;
    subtex->right = (float)SMDH_ICON_WIDTH / (float)SMDH_TEX_DIM;
    subtex->bottom = 1.0f - (float)SMDH_ICON_HEIGHT / (float)SMDH_TEX_DIM;

    out->tex = tex;
    out->subtex = subtex;
    return true;
}

/* UTF16 (fixed-length field) -> description/publisher UTF8 string.
 * Surrogates are dropped: codecvt aborts (no exceptions in devkitARM). */
static std::string smdh_title_to_utf8(const u16 *src, std::size_t max_chars) {
    std::u16string clean;
    for (std::size_t i = 0; (i < max_chars) && src[i]; i++) {
        u16 c = src[i];
        if ((c >= 0xD800) && (c <= 0xDFFF))
            continue;
        clean.push_back((char16_t)c);
    }

    return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> {}.to_bytes(clean);
}

void SMDH::Pump(const std::vector<std::u16string> &paths) {
    /* Control-phase only: this stays true because the only caller runs
     * right after C2D::Render (frame ended), never mid-frame. */
    smdh_frame++;
    smdh_free_garbage();

    /* Refresh LRU stamps for everything asked for. */
    for (const std::u16string &path : paths)
        for (SmdhNode &node : smdh_nodes)
            if (node.path.compare(path) == 0)
                node.last_use = smdh_frame;

    smdh_evict_lru();

    /* Decode one missing entry per frame (budget). Retry stale failures
     * only after the retry window passes (e.g. files arriving via the
     * netloader). */
    for (const std::u16string &path : paths) {
        std::size_t node_index = smdh_nodes.size();
        for (std::size_t i = 0; i < smdh_nodes.size(); i++)
            if (smdh_nodes[i].path.compare(path) == 0) {
                node_index = i;
                break;
            }

        if (node_index < smdh_nodes.size()) {
            SmdhNode &node = smdh_nodes[node_index];
            bool retry_due = (node.fail_stamp != 0) &&
                ((u32)(smdh_frame - node.fail_stamp) >= SMDH_FAIL_RETRY_FRAMES);
            if (!retry_due)
                continue; /* healthy cache hit or still inside wait window */

            /* Expire the stub and re-decode. */
            smdh_retire(node);
            smdh_nodes.erase(smdh_nodes.begin() + node_index);
            break;
        }

        SmdhNode fresh;
        fresh.path = path;
        fresh.last_use = smdh_frame;
        fresh.fail_stamp = 0;
        fresh.meta.has_icon = false;
        fresh.meta.icon.tex = nullptr;
        fresh.meta.icon.subtex = nullptr;

        Smdh smdh;
        bool decoded = smdh_fetch(path, &smdh) && (smdh.header.magic == SMDH_MAGIC);
        if (decoded) {
            fresh.meta.name = smdh_title_u16(smdh.applicationTitles[1].shortDescription, 0x40);
            fresh.meta.description = smdh_title_to_utf8(smdh.applicationTitles[1].longDescription, 0x80);
            fresh.meta.publisher = smdh_title_to_utf8(smdh.applicationTitles[1].publisher, 0x40);

            fresh.meta.has_icon = smdh_make_icon(&fresh.meta.icon, &smdh);
        }

        if (decoded && !fresh.meta.has_icon)
            Log::Error("SMDH: icon build failed\n");

        if (!decoded || !fresh.meta.has_icon)
            fresh.fail_stamp = smdh_frame;

        smdh_nodes.push_back(fresh);
        return; /* one decode budget per frame */
    }
}

bool SMDH::Peek(const std::u16string &path, SMDHMeta **out) {
    *out = nullptr;

    for (SmdhNode &node : smdh_nodes) {
        if (node.path.compare(path) == 0) {
            *out = &node.meta;
            return true;
        }
    }

    return false;
}

void SMDH::Reset(void) {
    for (SmdhNode &node : smdh_nodes)
        smdh_retire(node);

    smdh_nodes.clear();
    smdh_free_garbage();
}
