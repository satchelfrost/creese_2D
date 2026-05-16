# Reese and Cris Game Jam

A quick weekend game jam between friends. The ramdomly generated theme is **sponge hurried escape**.

The game will be built using Creese 2D software renderer.

## Creese 2D

A simple 2D software renderer with a few extra facilities.

## Getting Started

As of right now, Creese 2D can only build on Linux (however it can target windows).

First, build the build system [nob](https://github.com/tsoding/nob.h):

```bash
gcc -o nob nob.c
```

Then use nob to build the examples

```bash
./nob
```

Use `--help` to list other commands.
You can run the example from the base directory e.g:

```bash
./build/linux/example_circle
```

## Targeting Windows

First you must install mingw and wine e.g.:

```bash
sudo apt install gcc-mingw-w64-x86-64
sudo apt install wine
```

Then you can build examples by specifying the target e.g.:

```bash
./nob --target windows
```

Then you should be able to run them with wine (or simply run them on windows):

```bash
wine ./build/windows/example_circle.exe
```

## Game Jam Specific Commands

Run game after building (linux only):

```bash
./nob --run
```

Launch debugger (gf2 required) after building:

```bash
./nob --debug
```

To install gf2 go to https://github.com/nakst/gf and build from source, then put gf2 in your path.

Build examples (no longer on by default):

```bash
./nob --examples
```

Since the examples do take a second to build, we now make this an explicit command.
