## Installation
---
This game needs **SDL3 development libraries**, **GCC** (or Clang), **`make`**, and **`pkg-config`**. If you encounter any errors in the steps, I will attend to it ASAP and update the README to match the best way, I will specify which route I used to work on this project below.
### Windows
---
**Option A — MSYS2 (I used this option):**
1. Install [MSYS2](https://www.msys2.org/) and open the **UCRT64** terminal.
2. Update packages: `pacman -Syu` (close and reopen the terminal, then `pacman -Su` again).
3. Install dependencies:
```bash
   pacman -S mingw-w64-ucrt-x86_64-gcc \
             mingw-w64-ucrt-x86_64-make \
             mingw-w64-ucrt-x86_64-pkgconf \
             mingw-w64-ucrt-x86_64-sdl3
```

**Option B — WSL2: UNVERIFIED BUT RECOMMENDED**
I'm an avid WSL2 User, I highly recommend it esp if your system is not memory constrainted.
  
Install Windows Subsystem for Linux and follow the Linux instructions below.
https://learn.microsoft.com/en-us/windows/wsl/install

**Option C — vcpkg + Visual Studio: UNVERIFIED**
```bash
git clone https://github.com/microsoft/vcpkg
cd vcpkg

./bootstrap-vcpkg.bat
./vcpkg install sdl3
./vcpkg integrate install
```
Then build with `cl` from a Developer Command Prompt (you'd need to adapt the Makefile for MSVC or use CMake).

**Option D — manual install: UNVERIFIED**
Download the latest SDL3 Windows development package from [libsdl.org](https://github.com/libsdl-org/SDL/releases), extract, and point your compiler at the `include/` and `lib/` directories manually.
### Linux
---
**Ubuntu / Debian** (22.04+, may need a PPA on older releases since SDL3 is newer): UNVERIFIED
```bash
sudo apt update
sudo apt install build-essential pkg-config libsdl3-dev
```
  
If `libsdl3-dev` isn't available in your distro's repos yet, build from source (instructions at the bottom).

**Fedora:** UNVERIFIED

```bash
sudo dnf install gcc make pkgconf-pkg-config SDL3-devel
```

**Arch Linux / Manjaro:** UNVERIFIED
  
```bash
sudo pacman -S base-devel pkgconf sdl3
```

**openSUSE:** UNVERIFIED

```bash
sudo zypper install gcc make pkg-config SDL3-devel
```
### macOS UNVERIFIED
---
**Option A — Homebrew (recommended):**
```bash
brew install sdl3 pkg-config
```

GCC and Make come with Xcode Command Line Tools: 
```bash
xcode-select --install
```
**Option B — MacPorts:** UNVERIFIED
```bash
sudo port install libsdl3 pkgconfig
```
### Building SDL3 from source (universal fallback) or you're feeling a little brave.
---
If your package manager doesn't have SDL3 yet:

```bash
git clone https://github.com/libsdl-org/SDL.git
cd SDL
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
sudo make install
sudo ldconfig   # Linux only
```
This requires `cmake` and a C compiler. The install puts SDL3 in `/usr/local/` by default, where `pkg-config` will find it.
### Verify the install
---
```bash
pkg-config --modversion sdl3
```
Should print a version number (3.4.x or higher).
### Build and run this project
---
```bash
cd tictactoe
make
./tictactoe
```
# ABOUT THE PROJECT
---
Hi, my name is Kian, you may refer to me as Kianu if you prefer.

I have a bit of a thing for C, the idea of constraining yourself to a slightly minimalist mindset appeals to me, so I have been dabbling in it for a while, contemplating making a game. After trying out Vulkan for a while and getting the rendering there working, which trust me that took a while even if it was just following a tutorial, I decided I want to try something without a set up tutorial to make. Thus I created this project, Tic Tac C! The idea is to take a minimal interface as it's a home project and build out all the systems and subsystems myself. I'm using the Simple DirectMedia Layer 3.0 project (https://wiki.libsdl.org/SDL3/FrontPage), check them out if you're interested.  

If you find errors with anything, I'm only human. Please make a pull request and I'll go over it, whether its bad practice or some bug or the whatever. I hope you enjoy!

### NEXT STEPS
---
The reason I'm publishing this project now is because I wanted to give it a solid base state to build the rest of the project out.

In the future I have planned to implement the following

# Roadmap
### Prototype (DONE ✅)
---
- [x] Window, renderer, logical 640×360 with integer scaling
- [x] Game state types (`Cell`, `Player`, `Board`)
- [x] 3×3 grid rendering
- [x] Mouse click → piece placement
- [x] Turn alternation
- [x] Win detection (rows, cols, diagonals)
- [x] Draw detection
- [x] Game-over console message
- [x] Play again (R key) / quit (ESC)
- [x] Click bounds bug fixed
- [x] Mouse button filter (left-click only)

NEXT UP (TOP -> BOTTOM)
---
- [ ] **Refactor render into per-screen functions** (`menu_render`, `game_render`)
- [ ] **Screen state machine** (enum: `SCREEN_MENU`, `SCREEN_PLAYING`, `SCREEN_GAME_OVER`)
- [ ] **SDL3_ttf integration** — link library, init, load font, render text-to-texture pipeline
- [ ] **Main menu** with TTF buttons (Play PvP / Play PvAI / Quit)
- [ ] **On-screen game-over text** (replace console `printf`)

### ACTUAL NEW CONTENT
- [ ] AI opponent (Math readings TBD)
- [ ] Variable board size (k×k with k-in-a-row)
- [ ] Power-ups 
- [ ] Score System
### POLISH
- [ ] Asset Development (Aseprite → PNG → SDL3_image)
- [ ] Audio: SFX 
- [ ] Settings screen (volume, board size, AI difficulty)
- [ ] Save high scores or stats to drive

### HYGIENE
- [x] README.md with screenshots
- [ ] `.gitignore` for build artifacts
- [ ] License file (MIT or whatever fits)
- [ ] Cross-platform install instructions (✅ have draft)
- [ ] Add sanitizer flags to Makefile dev target (`-fsanitize=address,undefined`)
- [ ] Add release target with `-O2 -mwindows` (hide console for shipping)

### EXTRA
- [ ] Linux Build
- [ ] Replay system (record/playback game moves)
- [ ] Online multiplayer (lol, way later)

# Credits 

I'd like to dedicate this space in my project to recognize the contributions and effort from one of my dearest friends, Sarah. With your faith and encouragement I've been able to actualize a small bit of this project I've wanted to do for a while, and once it's done this will have been there since the start. As you're a woman in STEM that I've always admired, I wanted to highlight you in my space so that others can see what a cool cat you are (https://github.com/qymmore) here's to a job in our futures. 

I'd also like to say thanks to my closest friends from school and digitally across discord and other pockets of the internet, many of which have come directly into my life. I love you all too. 

And of course to my beloved animals who I've wanted to immortalize too, my cats, my birds and even my fish. The joy of caring for you is a lovely respite from my screens. 