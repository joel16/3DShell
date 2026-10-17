#ifndef _3D_SHELL_SMDH_H
#define _3D_SHELL_SMDH_H

#include <3ds.h>
#include <citro2d.h>
#include <string>
#include <vector>

#define SMDH_ICON_WIDTH 48
#define SMDH_ICON_HEIGHT 48
#define SMDH_TEX_DIM 64

typedef struct SMDHMeta {
    std::u16string name;   /* UTF16 as stored (no codecvt needed) */
    std::string description;
    std::string publisher;
    C2D_Image icon;
    bool has_icon;
} SMDHMeta;

namespace SMDH {
    /* Decode one missing .3dsx icon from the request list — call from the
     * control phase ONLY (never inside a C3D frame: GX_DisplayTransfer +
     * gspWaitForPPF are not allowed mid-frame). */
    void Pump(const std::vector<std::u16string> &paths);
    /* Read a cached result — safe to call during draw. */
    bool Peek(const std::u16string &path, SMDHMeta **out);
    void Reset(void);
}

#endif
