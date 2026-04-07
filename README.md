# Creese 2D

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
