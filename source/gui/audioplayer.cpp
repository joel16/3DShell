#include <cstring>

#include "audio.h"
#include "config.h"
#include "fs.h"
#include "gui.h"
#include "textures.h"
#include "touch.h"
#include "utils.h"

namespace GUI {
    typedef enum {
        STATE_NONE,
        STATE_REPEAT,
        STATE_SHUFFLE
    } AudioState;
    
    constexpr u32 guiTextColour = C2D_Color32(255, 255, 255, 255);
    constexpr u32 guiMusicGenreColour = C2D_Color32(97, 97, 97, 255);
    constexpr u32 guiMusicBgSeperatorColour = C2D_Color32(34, 41, 48, 255);

    static std::vector<FS_DirectoryEntry> entries;

    static void GetAudioPosition(char* out, size_t outSize, u64 seconds) {
        int h = seconds / 3600;
        int m = (seconds % 3600) / 60;
        int s = seconds % 60;
        
        if (h > 0) {
            std::snprintf(out, outSize, "%02d:%02d:%02d", h, m, s);
        }
        else {
            std::snprintf(out, outSize, "%02d:%02d", m, s);
        }
    }
    
    static int GetNextIndex(int current, AudioState state, bool forward) {
        if (entries.empty()) {
            return 0;
        }

        if (state == STATE_SHUFFLE) {
            int next = current;

            if (entries.size() > 1) {
                while (next == current) {
                    next = rand() % entries.size();
                }
            }

            return next;
        }

        if (state == STATE_REPEAT && !forward) {
            return current;
        }

        int next = forward ? current + 1 : current - 1;

        if (next >= (int)entries.size()) {
            return 0;
        }

        if (next < 0) {
            return entries.size() - 1;
        }
        
        return next;
    }

    void DisplayAudioPlayer(const char *initialPath) {
        FS::GetDirList(cfg.cwd, entries, FileTypeAudio);
        std::string currentPath = initialPath;
        AudioState state = STATE_NONE;
        bool quit = false;

        // Find initial selection index
        int selection = 0;
        std::string initialName = FS::GetFilename(initialPath);
        for (size_t i = 0; i < entries.size(); i++) {
            char entryName[256];
            Utils::UTF16ToUTF8(reinterpret_cast<u8*>(entryName), reinterpret_cast<const u16*>(entries[i].name), sizeof(entryName));

            if (strcasecmp(entryName, initialName.c_str()) == 0) {
                selection = i;
                break;
            }
        }
        
        while (aptMainLoop() && !quit) {
            Audio::Init(currentPath.c_str());
            
            char playbackLenString[17], playbackPosString[17];
            GetAudioPosition(playbackLenString, sizeof(playbackLenString), Audio::GetLengthSeconds());

            // Cache strings and dimensions once per song
            char titleUpper[64] = {0}, artistUpper[64] = {0}, filenameUpper[128] = {0};
            std::strncpy(titleUpper, metadata.title, 63);
            std::strncpy(artistUpper, metadata.artist, 63);
            std::strncpy(filenameUpper, FS::GetFilename(currentPath.c_str()), 127);
            
            strupr(titleUpper);
            strupr(artistUpper);
            strupr(filenameUpper);

            float playbackLenWidth = 0, titleHeight = 0;
            GUI::GetTextDimensions(0.45f, &playbackLenWidth, nullptr, playbackLenString);
            GUI::GetTextDimensions(0.5f, nullptr, &titleHeight, filenameUpper);

            bool locked = false;
            bool forceNext = false;

            aptSetSleepAllowed(false);
            
            while (aptMainLoop()) {
                GUI::Begin(guiBgColourTop[cfg.theme], guiBgColourBottom[cfg.theme]);
                
                GUI::DrawImage(defaultArtworkBlur, 0, 0);
                GUI::DrawRect(0, 0, 400, 18, guiMusicGenreColour);
                GUI::DrawRect(0, 55, 400, 2, guiMusicBgSeperatorColour);
                GUI::DisplayStatusBar();
                
                if (metadata.hasMeta && titleUpper[0] != '\0') {
                    if (artistUpper[0] != '\0') {
                        GUI::DrawText(5, 22, 0.5f, guiTextColour, titleUpper);
                        GUI::DrawText(5, 38, 0.45f, guiTextColour, artistUpper);
                    }
                    else {
                        GUI::DrawText(5, ((37 - titleHeight) / 2) + 18, 0.5f, guiTextColour, titleUpper);
                    }
                }
                else {
                    GUI::DrawText(5, ((37 - titleHeight) / 2) + 18, 0.5f, guiTextColour, filenameUpper);
                }
                
                if (metadata.image.tex) {
                    GUI::DrawImageScale(metadata.image, 0, 57, (175.0f / metadata.image.subtex->width), (175.0f / metadata.image.subtex->height));
                }
                else {
                    GUI::DrawImage(icAlbumLg, 0, 57);
                }
                
                GetAudioPosition(playbackPosString, sizeof(playbackPosString), Audio::GetPositionSeconds());
                GUI::DrawText(185, 205, 0.45f, guiTextColour, playbackPosString);
                GUI::DrawText(392 - playbackLenWidth, 205, 0.45f, guiTextColour, playbackLenString);
                
                if (Audio::GetLength() > 0) {
                    GUI::DrawRect(185, 220, 207, 2, guiMusicGenreColour);
                    float progress = (static_cast<float>(Audio::GetPosition()) / static_cast<float>(Audio::GetLength()));
                    GUI::DrawRect(185, 220, progress * 207.0f, 2, guiTextColour);
                }
                
                C2D_SceneBegin(GUI::GetRenderTarget(TARGET_BOTTOM));
                GUI::DrawImage(icMusicBgBottom, 0, 0);
                GUI::DrawImage(Audio::IsPaused() ? btnPlaybackPlay : btnPlaybackPause, 138, 100);
                GUI::DrawImage(btnPlaybackRewind, 58, 100);
                GUI::DrawImage(btnPlaybackForward, 218, 100);
                GUI::DrawImage(state == STATE_SHUFFLE ? btnPlaybackShuffleOverlay : btnPlaybackShuffle, 80, 140);
                GUI::DrawImage(state == STATE_REPEAT ? btnPlaybackRepeatOverlay : btnPlaybackRepeat, 210, 140);

                GUI::End();
                
                hidScanInput();
                Touch::Update();
                u32 kDown = hidKeysDown();

                if (kDown & KEY_B) {
                    quit = true;
                    break;
                }

                if (kDown & KEY_START) {
                    locked = !locked;
                }

                // Play/Pause
                if ((kDown & KEY_A) || (Touch::Rect(138, 100, 45, 45) && (kDown & KEY_TOUCH))) {
                    Audio::Pause();
                }
                
                // Toggles
                if ((kDown & KEY_X) || (Touch::Rect(80, 140, 30, 30) && (kDown & KEY_TOUCH))) {
                    state = (state == STATE_SHUFFLE) ? STATE_NONE : STATE_SHUFFLE;
                }
                if ((kDown & KEY_Y) || (Touch::Rect(210, 140, 30, 30) && (kDown & KEY_TOUCH))) {
                    state = (state == STATE_REPEAT) ? STATE_NONE : STATE_REPEAT;
                }

                // Navigation
                bool next = (kDown & KEY_RIGHT || kDown & KEY_R || (Touch::Rect(218, 100, 45, 45) && (kDown & KEY_TOUCH)));
                bool prev = (kDown & KEY_LEFT || kDown & KEY_L || (Touch::Rect(58, 100, 45, 45) && (kDown & KEY_TOUCH)));

                if (!playing && !Audio::IsPaused()) {
                    forceNext = true;
                }

                if (!locked && (next || prev || forceNext)) {
                    selection = GetNextIndex(selection, state, (next || forceNext));
                    char nextPath[1024];
                    FS::GetUTF8Path(nextPath, sizeof(nextPath), entries[selection].name);
                    currentPath = nextPath;
                    break;
                }
            }

            Audio::Exit();
            aptSetSleepAllowed(true);
        }
    }
}
