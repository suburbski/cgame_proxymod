# mDd client Proxymod:

This proxymod should help players to train their abilities for the Quake III Arena modification DeFRaG. It adds hud elements to the standard defrag hud.

## Installation
Quick and easy:
  1. Find the right *cgame* platform specific binary in the [latest release](../../releases/latest). To know which binary to choose, run `\version` in your engine's console. This will tell you *Windows vs Linux* and *32-bit vs 64-bit*.

|             | **32-bit** | **64-bit** |
| :---------: | :--------: | :--------: |
| **Windows** | [cgamex86.dll](../../releases/latest/download/cgamex86.dll) | [cgamex86_64.dll](../../releases/latest/download/cgamex86_64.dll) |
|  **Linux**  | [cgamei386.so](../../releases/latest/download/cgamei386.so) | [cgamex86_64.so](../../releases/latest/download/cgamex86_64.so)   |

  2. Download and copy this binary into the *defrag* folder of your Quake III Arena directory.
  3. Put `seta vm_cgame 0` in your config file (e.g. *defrag/autoexec.cfg*).

You've correctly installed the proxymod if you load your favorite map and you see the following colored text in the console: `[mDd] cgame-proxy: x.x.x`.

## Changelog
Please see [CHANGELOG](CHANGELOG.md) for notable changes between [releases](../../releases).

## Configuration
All the commands and cvars start with `mdd_`:

```
  mdd_hud
  mdd_version

  mdd_fov

  mdd_ammo
  mdd_ammo_graph_xywh
  mdd_ammo_text_xh
  mdd_ammo_text_rgba

  mdd_cgaz;
  mdd_cgaz_trueness;
  mdd_cgaz_yh;
  mdd_cgaz_rgbaNoAccel;
  mdd_cgaz_rgbaPartialAccel;
  mdd_cgaz_rgbaFullAccel;
  mdd_cgaz_rgbaTurnZone;

  mdd_gl_path_draw
  mdd_gl_path_rgba
  mdd_gl_path_preview_draw
  mdd_gl_path_preview_rgba

  mdd_jump
  mdd_jump_maxDelay
  mdd_jump_graph_xywh
  mdd_jump_graph_rgba
  mdd_jump_graph_rgbaOnGround;
  mdd_jump_graph_rgbaPreJump;
  mdd_jump_graph_rgbaPostJump;
  mdd_jump_graph_outline_w;
  mdd_jump_graph_outline_rgba;
  mdd_jump_text_xh
  mdd_jump_text_rgba

  mdd_rl_target_draw
  mdd_rl_target_shader
  mdd_rl_target_size
  mdd_rl_path_draw
  mdd_rl_path_rgba

  mdd_sound_local_only

  mdd_snap
  mdd_snap_trueness
  mdd_snap_speed
  mdd_snap_yh
  mdd_snap_def_rgba
  mdd_snap_alt_rgba
  mdd_snap_hl_def_rgba
  mdd_snap_hl_alt_rgba
  mdd_snap_45_def_rgba
  mdd_snap_45_alt_rgba

  mdd_timer
  mdd_timer_xywh
  mdd_timer_item_w
  mdd_timer_item_rgba
  mdd_timer_gb_rgba
  mdd_timer_outline_w
  mdd_timer_outline_rgba
```
Default values can be shown by typing the cvar into the console (e.g. `mdd_ammo_graph_xywh`) followed by pressing enter. Note that some cvars end with `_w`, `_xh` or even `_xywh`. Here `w` stands for width, `h` for height and `x` and `y` for the coordinates. It goes without saying that multiple values are separated by spaces.

## Bitset
There are a few cvars have binary-literals as default values (e.g. `mdd_ammo`). These start with `0b` followed by a sequence of `1`'s and `0`'s.

```
mdd_ammo 0b X X X X
            | | | |
            | | | + - draw
            | | + - - no weapon -> no draw
            | + - - - gun
            + - - - - 3D
```
```
mdd_cgaz 0b X
            | 
            + - draw
```
```
mdd_cgaz_trueness 0b X X X
                     | | |
                     | | + - jump/crouch influence
                     | + - - CPM air control zones
                     + - - - ground
```
```
mdd_snap 0b X X X X X
            | | | | |
            | | | | + - normal
            | | | + - - highlight active
            | | + - - - 45deg shift
            | + - - - - blue/red (min/max accel)
            + - - - - - height
```
```
mdd_snap_trueness 0b X X X
                     | | |
                     | | + - jump/crouch influence
                     | + - - CPM air control zones
                     + - - - ground (deprecated)
```
Note that it's not necessary to have the same number of `1`'s and `0`'s as there are *different options*, or even use the binary representation. You can still use the good old decimal equivalent (or the octal and hexadecimal representation to impress your friends).

## Examples

`mdd_ammo 0b1101` will:
  * draw the hud
  * draw the ammo/gun even if you don't have the weapon
  * draw the gun instead of the ammo
  * draw the 3D models instead of the 2D models

So if you want to have 2D models, simply use `mdd_ammo 0b0101`.

`mdd_cgaz_trueness 0b110` will:
  * not show jump/crouch influence
  * show correct air control zones in CPM
  * show correct zones when walking

## Building
The proxymod is written in C and uses CMake to control the building process.
1. Create a separate `build` directory (keeps your repository clean).
   ```
   $ mkdir build
   $ cd build
   ```
2. Generate input files for a native build system.
   To generate standard Makefiles for a 32 bit release build on Windows with gcc, do:
   ```
   $ cmake                                     \
       -DBINARY_NAME=cgamex86                  \
       -DCMAKE_BUILD_TYPE=Release              \
       -DCMAKE_C_COMPILER=gcc                  \
       -DCMAKE_C_FLAGS="${CMAKE_C_FLAGS} -m32" \
       -DCMAKE_INSTALL_PREFIX=/path/to/quake3/ \
       -DCMAKE_POSITION_INDEPENDENT_CODE=FALSE \
       -DCMAKE_SHARED_LIBRARY_SUFFIX_C=.dll    \
       -DFORCE_COLORED_OUTPUT=TRUE             \
       ..
   ```
   To generate Ninja files for a 64 bit debug build on Linux with clang, do:
   ```
   $ cmake -G Ninja                            \
       -DBINARY_NAME=cgamex86_64               \
       -DCMAKE_BUILD_TYPE=Debug                \
       -DCMAKE_C_COMPILER=clang                \
       -DCMAKE_C_FLAGS="${CMAKE_C_FLAGS} -m64" \
       -DCMAKE_INSTALL_PREFIX=/path/to/quake3/ \
       -DFORCE_COLORED_OUTPUT=TRUE             \
       ..
   ```
   Note that the binary name for a 32 bit Windows build is `cgamex86`, while `cgamei386` on Linux. The binary name for a 64 bit build, however, is `cgamex86_64` on both Windows and Linux.
3. Build the source code.
   ```
   $ cmake --build .
   ```
   If you specified an install prefix in step 2, you can build and install the new binary into the `defrag` subdirectory.
   ```
   $ cmake --build  . -- install
   ```
4. Profit.
