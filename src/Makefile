CC=emcc
CFLAGS=-O2 -s USE_SDL=2 -s USE_WEBGL2=1 -s USE_GLFW=3 --preload-file data
FILES=main.c
BUILD=index.html
BDIR=build

SRC=main.c shader.c window.c camera.c texture.c

all: files proj

proj: $(OBJ)
		$(CC) -o $(BDIR)/$(BUILD) $(SRC) $(CFLAGS)

files:
		mkdir -p $(BDIR)