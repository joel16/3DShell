# 3DShell [![Build Status](https://travis-ci.org/joel16/3DShell.svg?branch=master)](https://travis-ci.org/joel16/3DShell) ![Github latest downloads](https://img.shields.io/github/downloads/joel16/3DShell/total.svg)

![3DShell Banner](http://i.imgur.com/Z2pzVVZ.png)


Purpose:
--------------------------------------------------------------------------------
3DShell (pronounced 3D-Shell) - is a multi-purpose file manager for the Nintendo 3DS that aims towards handling media files. Unlike [FBI](https://github.com/Steveice10/FBI) which specializes more in title management, 3DShell leans towards more of the file management features such as opening and managing various files types and includes the basic necessity of a standard file manager. This program's design elements are clearly inspired by CyanogenMod/LineageOs's built in file manager, and so all credits towards the deisgn go to the CyanogenMod/LineageOs contributors.


Current features:
--------------------------------------------------------------------------------
- Storage bar (at the very top, just beneath the current working directory).
- Precise battery percentage using mcu::hwc.
- Creating new folders and files.
- Renaming files/folders.
- File/folder deletion.
- Copy/Move files and folders.
- Multi-select items for delete/cut/copy (using Y button).
- FTP server (Press select or tap the ftp icon to toggle).
- Image preview (If the image is around 400 * 480 which is the size of both screens, the image will be split in half and displayed. Support for the following image formats -> BMP, GIF - non animated, JPG, PCX, PNG, PGM, PPM and TGA)
- Extract various archives such as ZIP, RAR, 7Z, ISO 9660, AR, XAR and others supported by libarchive.
- Searching for directories (allows you to quickly visit a directory by clicking the search icon on the top right (bottom screen).)
- File properties - lets you view info on current file/folder, such as size, modified time, parent folder etc.
- Screenshots - Press (L + R) -> (/screenshots/Screenshot_YearMonthDay-Num.bmp)
- Fast scroll - Use analog stick.
- File timestamps.
- Browsing CTRNAND and copying data to/from CTRNAND.
- Audio playback support for the following formats -> IT, OGG, MOD, MP3, S3M, WAV and XM.
- Dir list Sorting (alphabetical - ascending, alphabetical - descending, size - largest to smallest, and size - smallest to largest).
- Online updater (nightly/releases)

Building from source:
--------------------------------------------------------------------------------
1. Ensure you have the devkitPro, ctrulib and citro3D and citro2D installed correctly. Make sure you have [makerom](https://github.com/profi200/Project_CTR) and [bannertool](https://github.com/Steveice10/bannertool) installed into your path as well.

2. Install the following dependecies from [devkitPro's pacman](https://devkitpro.org/viewtopic.php?f=13&t=8702):
* `sudo dkp-pacman -Syu 3ds-dev --noconfirm --needed`
* `sudo dkp-pacman -Syu 3ds-curl --noconfirm --force`
* `sudo dkp-pacman -Syu 3ds-libogg --noconfirm`
* `sudo dkp-pacman -Syu 3ds-libvorbisidec --noconfirm`
* `sudo dkp-pacman -Syu 3ds-mpg123 --noconfirm`
* `sudo dkp-pacman -Syu 3ds-opusfile --noconfirm`
* `sudo dkp-pacman -Syu 3ds-flac --noconfirm`
* `sudo dkp-pacman -Syu 3ds-libarchive --noconfirm`

3. Clone the 3DShell repository:
```bash
git clone --recursive https://github.com/joel16/3DShell.git
```
4. Open the project diretory:
```bash
cd 3DShell/
```
5. Build the binary using make:
```bash
make
```

Credits:
--------------------------------------------------------------------------------
- deltabeard/MaK11-12 for the inital ctrmus code port.
- mtheall for ftpd.
- preetisketch for the banner.
- FrozenFire for the boot logo.
