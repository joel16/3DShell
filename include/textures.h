#pragma once

#include <citro2d.h>

constexpr int NUM_ICONS = 6;

extern C2D_Image fileIcon[NUM_ICONS], iconDir[2], wifiIcon[4], batteryIcon[6], batteryIconCharging[6], \
	iconCheck[2], iconUncheck[2], iconRadioOff[2], iconRadioOn[2], iconToggleOn[2], iconToggleOff, \
	dialog[2], optionsDialog[2], propertiesDialog[2], iconHome, iconHomeOverlay, \
	iconOptions, iconOptionsOverlay, iconSettings, iconSettingsOverlay, iconFTP, iconFTPOverlay, \
	iconSD, iconSDOverlay, iconSecure, iconSecureOverlay, iconSearch, iconNavDrawer, iconAction, iconBack, \
	btnPlaybackForward, btnPlaybackPause, btnPlaybackPlay, btnPlaybackRepeat, btnPlaybackRepeatOverlay, \
	btnPlaybackRewind, btnPlaybackShuffle, btnPlaybackShuffleOverlay, defaultArtworkBlur, icAlbumLg, icMusicBgBottom;

namespace Textures {
	u32 GetNextPowerOf2(u32 v);
	bool C3DTexToC2DImage(C2D_Image *out, u32 width, u32 height, u32 channels, u8 *buf);
	bool LoadImageJPEG(u8 *data, u64 size, C2D_Image *texture);
	bool LoadImagePNG(const u8 *data, u64 size, C2D_Image *texture);
	bool LoadImageFile(const char *path, C2D_Image *texture);
	void Free(C2D_Image *image);
    void Init(void);
    void Exit(void);
}
