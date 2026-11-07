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
<img width="1920" height="1080" alt="Unbenannt" src="https://github.com/user-attachments/assets/c6f8711c-4396-45c8-8fa6-4e6cbca15603" />
<img width="1920" height="1080" alt="Unbenannt" src="https://github.com/user-attachments/assets/4a9e5b0c-abaf-48b5-a64a-aad5d2ae9340" />
<img width="1440" height="2560" alt="image" src="https://github.com/user-attachments/assets/45e0d948-77c9-4430-b879-6e3f74ac5b13" />

doesn't do anything else
