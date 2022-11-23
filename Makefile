
all:
	$(error specify platform (linux|windows))

linux:
	g++ src/main.cpp src/renderer.cpp -o build/main.out -lSDL2 -lSDL2_ttf -lSDL2_image -std=c++20

windows:
	g++ src/main.cpp src/renderer.cpp -o build/main.exe -Iinclude -Llib -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -lSDL2_image -std=c++20

.PHONY: linux windows