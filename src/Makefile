CC=emcc
CFLAGS=-O2 -s USE_SDL=2 -s USE_WEBGL2=1 -s USE_GLFW=3 -s USE_FREETYPE=1 -s ALLOW_MEMORY_GROWTH=1 -lopenal --preload-file data
FILES=main.c
BUILD=index.html
BDIR=build

SRC=main.c shader.c window.c camera.c texture.c mesh.c model.c iqm.c scene.c text.c sound.c

all: files proj

proj: $(OBJ)
		$(CC) -o $(BDIR)/$(BUILD) $(SRC) $(CFLAGS)

files:
		mkdir -p $(BDIR)