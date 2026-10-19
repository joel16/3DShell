# UI Components & Screen Layout Report

Scope: current state of the component-based UI (`source/ui/` framework +
`source/shell/gui/screens/` application screens). Verified against the code on
this branch (`framework`).

## 1. Rendering pipeline

| Layer | Class / function | File |
|---|---|---|
| GPU init, render targets | `Services::Init()` creates `top_screen` and `bottom_screen` via `C2D_CreateScreenTarget(GFX_TOP/GFX_BOTTOM, GFX_LEFT)` | `source/shell/main.cpp` |
| Drawing primitives | `C2D::Text / Textf / Rect / Image / ImageScale / GetTextSize / Render` (thin wrappers over citro2d) | `source/ui/c2d_helper.cpp` |
| Frame loop | `UI::ScreenManager::Run()` | `source/ui/screen_manager.cpp` |
| Text glyph buffers | `static_buf / dynamic_buf / size_buf` (`C2D_TextBuf`, 4096 each) | `source/ui/c2d_helper.cpp` |

### Framebuffers

| Name | Target | Size | Cleared with |
|---|---|---|---|
| **gfx0** | `top_screen` (`GFX_TOP`) | 400x240 | `UI::Screen::TopBackground()` |
| **gfx1** | `bottom_screen` (`GFX_BOTTOM`) | 320x240 | `UI::Screen::BottomBackground()` |

Frame order inside `UI::ScreenManager::Run()` (every iteration):

1. `screen->OnFrame(delta_time)` — per-frame state update.
2. `C3D_FrameBegin(C3D_FRAME_SYNCDRAW)`.
3. `C2D_TargetClear(top_screen, screen->TopBackground())` and the same for the bottom target.
4. `C2D_SceneBegin(top_screen)` → `screen->DrawTop()`.
5. `C2D_SceneBegin(bottom_screen)` → `screen->DrawBottom()`.
6. `C2D::Render()` (clears text buffers + `C3D_FrameEnd`).
7. `hidScanInput()` → `screen->OnKeyHeld(kHeld)` → `screen->OnKeyDown(kDown)` → `screen->OnTouch(touch, touching)`.

## 2. Framework components (`source/ui/`)

| Component | Responsibility |
|---|---|
| `UI::Rect` | Plain x/y/w/h box + `Contains(touch)` hit-testing. |
| `UI::Control` | Base class: `Bounds`, `Visible`, `Enabled`, virtual `Draw()`, `OnKeyDown()`, `OnKeyHeld()`, `OnTouch()`. |
| `UI::ContainerControl` | Owns child controls, forwards draw/input, implements mouse-style touch capture (hit-test on touch-down, delivers `OnTouch(touch, true)` per held frame, `OnTouch(touch, false)` once on release). |
| `UI::Screen` | "Form": owns `TopPanel()` and `BottomPanel()` (`UI::ContainerControl`), virtual `OnFrame(delta)`, `DrawTop()`, `DrawBottom()`, `OnKeyDown/OnKeyHeld/OnTouch`, `TopBackground/BottomBackground` colours. |
| `UI::ScreenManager` | Stack of screens (`Push`/`Pop` with deferred destruction), owns the main loop, input routing, `Exit()`, `DeltaTime()`, `IsCurrent()`. |
| `UI::Label` | Static text (text, position, size, colour). |
| `UI::Button` | Rect + centered text + press feedback + `Click` callback. |
| `UI::ScrollBar` | Vertical track/thumb; `SetRange(item_count, page_size)`, `Value()` = first visible row, finger drag. |
| `UI::Dialog` | Panel with title bar; base for popups (derives from `ContainerControl`). |
| `UI::FlowLayout` | Horizontal packer: places visible controls left-to-right from an anchor or right-to-right ending at an anchor (right-aligned groups); invisible controls are skipped, so groups re-pack when one is hidden. |

## 3. Shared chrome (application controls)

Both are real controls now, in `source/shell/controls/` — application-level
because they depend on app assets and hardware services:

### 3.1 Status bar — `GUI::StatusBar` (gfx0 only, y = 0..15)

Class: `source/shell/controls/status_bar.h`. Drawn as part of each screen's
top chrome (`FileBrowserScreen`, `TextReaderScreen`).

| Element | Position | Drawn with | Assets |
|---|---|---|---|
| Bar background | rect (0, 0, 400, 15) | `C2D::Rect`, `STATUS_BAR_LIGHT` / `STATUS_BAR_DARK` | — |
| Clock ("H:MM AM/PM") | left, x = 5 | `std::time` + `snprintf` + `C2D::Text`, white | — |
| Battery percentage | right, x = 395 - width | `MCUHWC_GetBatteryLevel` + `C2D::Text` | — |
| Battery icon | left of the percentage | `C2D::Image(battery_icons[level])`, level from `PTMU_GetBatteryLevel` (0..5) | `battery_icons[6]` |
| Wi-Fi icon | left of the battery icon | `C2D::Image(wifi_icons[strength])`, from `osGetWifiStrength` (0..3) | `wifi_icons[4]` |

### 3.2 Touch bar — `GUI::TouchBar` (gfx1 only, y = 0..20)

Class: `source/shell/controls/touch_bar.h`. A `UI::ContainerControl` holding
six `GUI::TouchButton` children (icon button with light/dark/active variants
and a `Click` event), laid out by `UI::FlowLayout` in two groups: the LEFT
group flows from the left edge, the RIGHT group is anchored to the right
edge. Invisible buttons are skipped by the layout, so the group re-packs
(no dead gaps). Hit testing and touch capture are handled by the container
tree — no hard-coded zones.

| Button | Group | Event | Overlay (active) when |
|---|---|---|---|
| Home | LEFT | `Home().Click` | `section == SECTION_BROWSER` |
| Options | LEFT | `Options().Click` | `section == SECTION_ACTIONS` |
| Settings | LEFT | `Settings().Click` | `section == SECTION_SETTINGS` |
| SD card | RIGHT | `SD().Click` | `archive == sdmc_archive` |
| NAND (secure) | RIGHT | `Secure().Click` | `archive == nand_archive` (needs `cfg.dev_options` to switch) |
| Search | RIGHT | `Search().Click` | — |

`TouchBar::SetSection()` controls which button shows its overlay variant;
screens wire the `Click` events to their own actions (the browser pushes
dialogs / switches archives, the text reader closes the editor).

## 4. Screens

Every screen is a class deriving from `UI::Screen` in
`source/shell/gui/screens/`. `UI::ScreenManager` keeps them in a stack; the
browser is the root and pushes dialogs with `OpenDialog()`.

### 4.1 FileBrowserScreen (root screen)

**gfx0 (top)** — `FileBrowserScreen::DrawTop()`:

| Element | Position | Drawn by |
|---|---|---|
| Status bar (background, clock, battery, wifi) | y = 0..15 | `status_bar_.Draw()` — `GUI::StatusBar` (see 3.1) |
| Menu bar background | rect (0, 15, 400, 25) | `C2D::Rect`, `MENU_BAR_*` |
| Current directory path (truncated at 60 chars) | x = 5, inside menu bar | `C2D::Textf` |
| Storage usage bar (used vs total) | (5, ~28..30), width 390 | `C2D::Rect` x2 (`SELECTOR_*` track + `TITLE_*` fill) |
| "This is an empty directory" message | centered | `C2D::Text` (only when the listing is empty) |
| Entry rows (10 visible, `sel_dist` = 20, starting y = 40): selection highlight, check/uncheck mark, directory or file-type icon (SMDH icon scaled 48→16 for .3dsx), file name (truncated at 52 chars) | x = 0..395 | `FileBrowserScreen::DrawTop()` loop; icons from `icon_check*`, `icon_dir*`, `file_icons[]`, `SMDH::Peek` |
| Scroll indicator (current scroll window of the listing) | (395, 40, 4, 200) | `UI::ScrollBar scroll_bar_` member, driven with `SetRange(entries, 10)` + `SetValue(start)` |

**gfx1 (bottom)** — `FileBrowserScreen::DrawBottom()`:

| Element | Position | Drawn by |
|---|---|---|
| Touch bar (six buttons in LEFT/RIGHT layout groups) | y = 0..20 | `touch_bar_.Draw()` — `GUI::TouchBar` (see 3.2) |
| SMDH info panel for the selected .3dsx (icon, name, description, publisher) | (16, 26) onwards | `FileBrowserScreen::DisplaySmdhPanel()` + `FitText()` |
| Action grid 2x2 (New File, New Folder, Properties, Rename) + NetLoader button | grid centered at y = 80 | `FileBrowserScreen::DisplayActionGrid()` |
| Touch handling for the action grid | y = 80..140 | `FileBrowserScreen::HandleActionGrid()` (touch bar is handled by `TouchBar` itself, see 3.2) |

Hidden while a dialog is open (`dialog_open_`): the SMDH panel and the grid.

### 4.2 OptionsScreen (pushed by the browser)

Draws `owner_->DrawTop()/DrawBottom()` first (browser stays visible behind),
then its own layer on gfx1:

| Element | Position | Drawn by |
|---|---|---|
| Dialog texture ("Actions") | (54, 30) | `C2D::Image(options_dialog / options_dialog_dark)` |
| Title text | (61, 34) | `C2D::Text` |
| Selector highlight (row/column grid cursor) | cells at (56|160, 69|105|142) | `C2D::Rect`, `SELECTOR_*` |
| Page 1 entries: Properties, Copy/Paste, Delete, Refresh, Move/Paste, More... | fixed text positions | `C2D::Text` |
| Page 2 ("More..."): New folder, Rename, New file | same positions | `C2D::Text` |
| CANCEL button | bottom-right of the dialog | `C2D::Text` + selector rect |

Keyboard: D-Pad moves the cursor, A activates, B goes back. Touch: same cells
via `UI::Rect::Contains`. "Properties"/"Delete" push `PropertiesScreen` /
`DeleteScreen`; Refresh/Copy/Move/Rename completion pops the screen.

### 4.3 PropertiesScreen (pushed from browser or options)

gfx0: browser behind (`owner_->DrawTop()`).
gfx1 — `PropertiesScreen::DrawBottom()`:

| Element | Position | Drawn by |
|---|---|---|
| Dialog texture | centered | `C2D::Image(properties_dialog*)` |
| Title "Properties" | top-left of the dialog | `C2D::Text` |
| `Parent:` (cwd, truncated at 22 chars) | (66, 57) | `C2D::Textf` |
| `Size:` (human readable) | (66, 73), files only | `Utils::GetSizeString` + `C2D::Textf` |
| OK button + highlight | bottom-right (253, 218) | `C2D::GetTextSize` + `C2D::Rect` + `C2D::Text` |

A/ B or touching OK closes the screen (`Pop()`).

### 4.4 DeleteScreen (pushed from browser or options)

gfx0: browser behind. gfx1 — `DeleteScreen::DrawBottom()`:

| Element | Position | Drawn by |
|---|---|---|
| Dialog texture + title "Delete" | centered | `C2D::Image` + `C2D::Text` |
| Prompt "Do you wish to continue?" | centered, y = 117 | `C2D::GetTextSize` + `C2D::Text` |
| YES / NO buttons with selector | (248|288, 159) | `C2D::Rect` + `C2D::Text` |

YES deletes the selection (single or multi-check via `Options::Delete`); NO
cancels. Left/Right or touch chooses; A confirms.

### 4.5 SettingsScreen (pushed from the browser)

gfx0: browser behind. gfx1 — `SettingsScreen::DrawBottom()`:

| Element | Position | Drawn by |
|---|---|---|
| Page menu bar + content panel | (0, 20, 400, 35) and (0, 55, 320, 185) | `C2D::Rect` |
| Back icon | (5, 25) | `C2D::Image(icon_back)` (sub-pages only) |
| Row selector | y = 55 + selection * 40 | `C2D::Rect` |
| **General page**: Sort by, Dark theme, Developer options, File filter (+ current value), Check for update, toggle icons | rows at y = 58/98/138/178/230 | `SettingsScreen::DisplayGeneral()` |
| **Sort page**: 4 radio options (Alphabetical asc/desc, Size desc/asc) | rows at y = 58/98/138/178 | `SettingsScreen::DisplaySort()` + `icon_radio_*` |
| **Update page**: Check for updates, About (version/author/assets) | rows at y = 58/98 | `SettingsScreen::DisplayUpdatePage()` |
| **Update popup** (when checking done): Error / Update Available / No Updates / Update Successful + YES/NO|OK | centered dialog texture | `SettingsScreen::DisplayUpdatePopup()` |

Toggle icons (`icon_toggle_*`) reflect `cfg.dark_theme` and `cfg.dev_options`;
the theme toggle re-applies the background colours every frame.

### 4.6 ImageViewerScreen (pushed by the browser)

Covers **both** screens (backgrounds forced to `BLACK_BG`; nothing of the
browser shows through):

| Element | Screen | Drawn by |
|---|---|---|
| Image in DEFAULT dimension (auto-scaled to 240 height, zoom 0.5x..2x, pan via C-Pad) | gfx0, centered | `ImageViewerScreen::DrawImageTop()` + `DrawImage()` (`C2D_DrawImage` with params) |
| Image in NINTENDO_SCREENSHOT dimension (432x528) | gfx0 (16,16) + gfx1 (56,272) | same |
| Image in DUAL_SCREEN dimension (400x480/482) | gfx0 (0,0) + gfx1 (40,240) | same |
| Properties popup (X): dialog texture, Name, Width, Height, OK | gfx1, centered | `ImageViewerScreen::DrawImageBottom()` |
| Zoom/pan controls | — | `ImageViewerScreen::Control()`: D-Pad/C-Stick zoom, C-Pad pan, SELECT resets, D-Left/Right or L/R cycles to the previous/next image in the directory |

### 4.7 TextReaderScreen (pushed by the browser)

gfx0 — `TextReaderScreen::DrawTop()`:

| Element | Position | Drawn by |
|---|---|---|
| Status bar + menu bar background | y = 0..15 / 15..40 | `status_bar_.Draw()` — `GUI::StatusBar` + `C2D::Rect` |
| File name title (truncated at 45 chars, `*` when modified) | (5, 40) | `Notepad::DisplayTop()` |
| Text lines (11 visible, 50 cols, horizontal/vertical scroll) | from (8, 60), line height 14.5 | `Notepad::DisplayTop()` loop |
| Cursor line highlight + blinking cursor (edit mode) | cursor line | `C2D::Rect` |
| Status line (L/C, View/Edit, modified `*`) | (5, 222) | `C2D::Textf` |
| Save feedback: "Saved" (green `SUCCESS_COLOUR`) / "Save failed" (red `ERROR_COLOUR`) | right of the status line (x = 395 - width) | `Notepad::DisplayTop()`, flashes for 1.5 s after `Notepad::Save()` |

gfx1 — `TextReaderScreen::DrawBottom()`:

| Element | Position | Drawn by |
|---|---|---|
| Touch bar (no active section) | y = 0..20 | `touch_bar_.Draw()` — `GUI::TouchBar`, `SECTION_NONE` |
| **View mode**: title "Text Reader", key hints (A:Edit X:Save B:Back Pad:Move), buttons Edit/Save/Back, `* unsaved` marker | (10, 30) onwards | `Notepad::DisplayBottom()` |
| **Edit mode**: `VirtualKeyboard::Display()` on-screen keyboard | lower area | `VirtualKeyboard::Display()` (`source/shell/osk/virtual_keyboard.cpp`) |
| Touch bar Home button closes the editor | y = 0..20, left zone | `TextReaderScreen` wires `touch_bar_.Home().Click` → `Notepad::Close()` |

### 4.8 Progress overlay (any screen, transfer-time only)

| Element | Screen | Drawn by |
|---|---|---|
| Dialog texture + title + message + progress bar (240 px, fill by ratio) | gfx1, centered | `GUI::ProgressBar()` (`source/shell/gui/gui.cpp`) — used by NetLoader (`GUI::DownloadProgressBar` thread) and the updater |

## 5. Component status / pending refactor

| Item | Today | Target |
|---|---|---|
| Dialog textures + selector rects | Direct `C2D::Image`/`C2D::Rect` calls in each screen | `UI::Dialog` / `UI::Button` controls (already in the framework) once theme colours are injectable |
| File list | Custom loop in `FileBrowserScreen::DrawTop()` | `ListView` control (framework candidate) |
| Browser god-struct `MenuItem` | Still shared by pointer with dialogs (entries, checked, texture) | Split into per-screen state models |
| Status bar / touch bar | `GUI::StatusBar` / `GUI::TouchBar` app controls, held as members by the screens | Move into the screens' `TopPanel()`/`BottomPanel()` containers for tree-driven drawing/routing |

## 6. Theme system

Every colour comes from `include/colours.h` (`*_LIGHT` / `*_DARK` pairs plus
`SUCCESS_COLOUR` / `ERROR_COLOUR`). `cfg.dark_theme` selects the pair at draw
time; `SettingsScreen` toggles it live and screens re-apply their background
colours in `OnFrame`, so the change is visible immediately.
