# Jw32 #

Win32 bindings for [Janet](https://janet-lang.org/).

This package is currently only intended to work as a support library for [Jwno](#).

## Dependencies ##

* Visual Studio 2022 (the Community version will do)
* Windows SDK >= 10.0.22621.0 (comes with Visual Studio)
* Janet >= 1.34.0 (compiled with the MSVC toolchain)
* [Jpm](https://janet-lang.org/docs/jpm.html)

## Compiling ##

1. Set the `JANET_SOURCE_PATH` environment variable to point to the Janet source tree;
2. Start an `x64 Native Tools Command Prompt for VS 2022`;
3. Inside the command prompt, run `jpm -l build`;
4. Check `build\jw32\` for built artifacts.

## Installing ##

To install this package locally, run `jpm -l install`.

To install it as a dependency for Jwno, run `jpm --tree=path\to\jwno\jpm_tree install`.
