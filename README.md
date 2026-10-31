# fullscreendigitalclockforwindows
copying justabigclock (https://justabigclock.com) for their night time clock except this is a local program. it's a fullscreen digital clock for windows.

- it supports the ISO8601 format (no time zone notation)
- it's not that resource hungry, about a megabyte of ram and .5% of CPU
- trying to keep in under 50 kilobytes
- config file can be opened by pressing e
- the window is resizable, but starts full screen (press alt+space for the menu to appear, choose resize)
- default font is atkinson hyperlegible next mono light (https://github.com/googlefonts/atkinson-hyperlegible-next-mono/raw/refs/heads/main/fonts/ttf/AtkinsonHyperlegibleMono-Light.ttf)
- g++ -O3 -Os -flto -s -ffunction-sections -fdata-sections -Wl,--gc-sections -Wl,--strip-all -Wl,--build-id=none fullscreendigitalclockforwindows.cpp -o fullscreendigitalclockforwindows.exe -luser32 -lgdi32
- https://winlibs.com/#download-release

doesn't do anything else
