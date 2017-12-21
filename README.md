# 3DShell ![Github latest downloads](https://img.shields.io/github/downloads/joel16/3DShell/total.svg)

![3DShell Banner](http://i.imgur.com/Z2pzVVZ.png)


Purpose:
--------------------------------------------------------------------------------
3DShell (pronounced 3D-Shell) - is a multi-purpose file manager GUI for the Nintendo 3DS. The program is currently in its early stages and lacks many features that are currently in development. More information will be given once the program matures. This program's design elements are clearly inspired by CyanogenMod's built in file manager, and so I take no credit for that.


Current features:
--------------------------------------------------------------------------------
- Storage bar (at the very top, just beneath the current working directory).
- Precise battery percentage using mcu::hwc
- Creating new folders
- Renaming files/folders
- File/folder deletion
- Copy/Cut files and folders
- FTP server
- Image preview (If the image is around 400 * 480 which is the size of both screens, the image will be split in half and displayed. Have a look at the screenshots below). Support for the following image formats. PNG, JPG, GIF[un-animated], (untested ->) HDR, PIC, PNM, PSD, TGA)
- Zip file extraction.
- Searching for directories (allows you to quickly visit a directory by clicking the search icon on the top right (bottom screen).)
- File properties - let's you view info on current file/folder.
- Screenshots - Press (L + R) -> (/screenshots/Screenshot_YearMonthDay-Num.bmp)
- Fast scroll - Use analog stick
- File's time-stamp
- Browsing CTRNAND
- MP3, vorbis (ogg), flac and wav playback support. (All thanks to deltabeard/MaK11-12)


Building from source:
--------------------------------------------------------------------------------
1. Ensure you have the devkitPro, ctrulib and citro3D installed correctly. Make sure you have [makerom and bannertool](https://github.com/Steveice10/buildtools/tree/master/3ds) in your path as well.

2. Install the following dependecies from [devkitPro's portlibs](https://github.com/devkitPro/3ds_portlibs):
`libogg` and `tremor - low mwmory branch`.

Then use the unofficial (fork) of portlibs [here](https://github.com/deltabeard/3ds_portlibs) to install libmpg123.

To compile mpg123, just run `make mpg123` in the 3ds_portlibs folder. This will compile all that is required of mpg123. Installing the library has to be done manually though, and can be done by running the following commands:
* cp mpg123-1.xx.x/src/libmpg123/.libs/libmpg123.a /opt/devkitpro/portlibs/armv6k/lib/
* cp mpg123-1.xx.x/src/libmpg123/libmpg123.la /opt/devkitpro/portlibs/armv6k/lib/
* cp mpg123-1.xx.x/libmpg123.pc /opt/devkitpro/portlibs/armv6k/lib/pkgconfig/
* cp mpg123-1.xx.x/src/libmpg123/mpg123.h /opt/devkitpro/portlibs/armv6k/include/
* cp mpg123-1.xx.x/src/libmpg123/fmt123.h /opt/devkitpro/portlibs/armv6k/include/

3. Clone the repo:
```bash
$ git clone --recursive https://github.com/joel16/3DShell.git
```
4. Build the program using make:
```bash
$ cd 3DShell/ && make
```
The binaries will be distributed in the "out" directory.


Credits:
--------------------------------------------------------------------------------
- deltabeard/MaK11-12 for sound support.
- mtheall for ftpd.
- Steveice10 for allowing me to use his screen.c C3D code as a template to build and make  modifications on.
- preetisketch for the banner.
- FrozenFire for the boot logo.


