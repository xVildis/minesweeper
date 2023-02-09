# Omavalintainen loppuprojekti

## MSYS2 / mingw64 setup

1. Download msys2 from https://www.msys2.org/
2. Install msys2 to c:/msys64
3. Install mingw64 toolchain by running `pacman -S --needed base-devel mingw-w64-x86_64-toolchain`
4. Add `C:\msys64\usr\bin` and `C:\msys64\mingw64\bin` to your PATH
5. call `make run`

## Emscripten build

Enable emscripten environment

`emcc -sUSE_SDL=2 -sUSE_SDL_IMAGE=2 -sUSE_SDL_TTF=2 -sSDL2_IMAGE_FORMATS=["png"] src/main.cpp src/renderer.cpp src/globals.cpp -o bin\emscripten\test.html --preload-file .\assets`
