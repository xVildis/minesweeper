
all:
	$(error specify platform (linux|windows))

linux:
	g++ src/main.cpp -o build/main.out -lSDL2 -std=c++20

windows:
	g++ src/main.cpp -o build/main.exe -Iinclude -Llib -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -std=c++20

.PHONY: linux windows