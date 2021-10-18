# Why?
It could potentially be useful as a tool for fixing *some* older homebrew/hacks that don't work outside of ZSNES, by serving as living-code documentation of [why](https://github.com/lifning/bZSNES/commit/e05af67ebdaf43e9f3a4b772d30587da1ebd32d3) they might be broken.
It's also possible that it could be useful to *run* such things on hardware that isn't capable of running 32-bit x86 code.
But mostly, because I found their joke hilarious ten years ago, and wanted to preserve it better than what little I was able to find on the web today.

# Building
In the `bsnes` subdirectory, run:
- `make library` to build the combination libsnes+libretro (yes, it still works in old SSNES builds and other libsnes frontends if you have some reason to want that).
- `make ui=ui-phoenix` to build the Phoenix/GTK+2 frontend.
- `make` to build the Qt5 frontend.

##### Posted by Near on April 1st, 2011:
> # About
> bZSNES is a portable ZSNES emulator for Windows, OS X and Linux; written in C++. It is licensed under the GNU GPLv2.
> 
> # Features
> bZSNES emulates almost all of the major features of ZSNES:
> - VRAM can be written at any time, even while screen is rendering
> - OAM and CGRAM can also be written at any time
> - DSP echo buffer writes do not interfere with APURAM
> - DRAM does not need to be periodically refreshed
> - H/DMA synchronization does not steal CPU time
> - auto joypad polling results can be read immediately
> - CPU multiplication and division register results can be read immediately
> - mid-scanline writes to PPU registers do not interfere with video output
> - PPU background and sprite layers can be individually toggled on and off
> - DSP voice channels can be individually toggled on and off
> - CPU speed can be adjusted from its official frequency
>
> ... and more!
> 
> # Compatibility
> bZSNES is sure to be highly compatible with software made for ZSNES, such as:
> - BS Zelda [English translation]
> - Bahamut Lagoon [English translation]
> - Sailor Moon: Another Story [English translation]
> - Super Mario World [Addmusic hacks]
>
> ... and much more!
> 
> # Look & Feel
> But it doesn´t stop there! bZSNES also emulates the look and feel of ZSNES. You can even theme bZSNES with your own colors and fonts!
