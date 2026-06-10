# Building Wagic

Wagic targets several platforms from one codebase. The game logic lives in
`projects/mtg/`, the engine layer in `JGE/`. Pick your platform below.

## Linux (SDL2) — primary desktop dev build

Requires the SDL2 port of the JGE layer (merged 2026; if your checkout
predates it, the Linux/SDL path targeted an unreleased SDL 1.3 snapshot and
did not compile).

Dependencies (Arch): `qt6-base sdl2-compat boost libjpeg-turbo libpng giflib glu zlib`
Dependencies (Debian/Ubuntu): `qt6-base-dev libsdl2-dev libboost-dev libboost-thread-dev libjpeg-dev libpng-dev libgif-dev libglu1-mesa-dev zlib1g-dev`

```bash
cd projects/mtg
qmake6 wagic-SDL.pro CONFIG+=debug -o Makefile.sdl
make -f Makefile.sdl -j$(nproc)
cd bin && ./wagic     # must run from bin/ (needs ./Res)
```

Notes:
- Always pass `-o Makefile.sdl`: a bare `qmake6` overwrites the tracked
  `projects/mtg/Makefile` (the PSP makefile).
- The C++ standard is pinned to `gnu++14` in `wagic-SDL.pro` (the code still
  uses `random_shuffle`/`bind2nd`/`ptr_fun`, removed in C++17).
- Release build: drop `CONFIG+=debug`. Debug builds include the test suite.

### Running the test suite (debug builds)

```bash
cd projects/mtg/bin
WAGIC_TESTSUITE=1 ./wagic        # runs headlessly, exits 0/1 with a summary
```

Results land in `User/test/results.html`. To run a subset, edit
`Res/test/_tests.txt` (restore it afterwards — it is a tracked file).

## Linux (Qt)

An alternative Qt front end exists:

```bash
cd projects/mtg
qmake -qt=qt5 wagic-qt.pro
make -j$(nproc)
```

Needs `qt5-qmake qtbase5-dev qtdeclarative5-dev qttools5-dev
qtmultimedia5-dev libqt5opengl5-dev` (Debian names). This is the
front end the old Travis CI built.

## Windows

Open `projects/mtg/mtg_vs2010.sln` in Visual Studio (the solution has been
imported into newer VS versions; AppVeyor built it this way). Build the
`wagic` project; run with the working directory set to a folder containing
`Res/`.

## Android

The in-tree Android project (`projects/mtg/Android/`) is the legacy
ant + NDK layout; the old CI used android-ndk-r22 with `ant`. Expect to
update SDK/NDK paths in `local.properties`. A modernization to Gradle is an
open task.

## PSP

The original target. Needs the PSP toolchain (`PSPDEV`/`PSPSDK` env vars,
pspsdk 0.11.x):

```bash
cd projects/mtg
make            # uses the tracked PSP Makefile
```

## PS Vita

A community port by Brendonm17 (github.com/Brendonm17/wagic-vita) builds
with VitaSDK + vitaGL via CMake. Forks that have merged it carry a
`BUILD_VITA.md` with the full recipe, including the required vitaGL version
pin.

## macOS / iOS

Historical Xcode projects exist under `projects/mtg/` but have not been
exercised recently; expect bitrot. Contributions welcome.
