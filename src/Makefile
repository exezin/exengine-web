# -- Dirs -- #
ifneq ($(USE_EMCC),1)
CC    =gcc
FLAGS =-g -lm -lstdc++ -lGL -lGLEW -lglfw -lfreetype -lopenal -I/usr/include/freetype2
else
CC    =emcc
FLAGS =-O2 -s USE_WEBGL2=1 -s USE_GLFW=3 -s USE_FREETYPE=1 -s ALLOW_MEMORY_GROWTH=1 -lopenal --preload-file data
endif

BDIR  =build
ODIR 	=obj
CFLAGS  =$(FLAGS)

# -- Files -- #
# deps
DEPS		=shader.h window.h camera.h texture.h mesh.h model.h iqm.h scene.h text.h sound.h

# srcs
_OBJ		=main.o shader.o window.o camera.o texture.o mesh.o model.o iqm.o scene.o text.o sound.o
OBJ 		=$(patsubst %,$(ODIR)/%,$(_OBJ))

all: files proj

# srcs
$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

proj: $(OBJ)
	$(CC) -o $(BDIR)/$@ $^ $(CFLAGS)
	@echo "**success**"

files:
	@echo $(USE_EMCC)
	mkdir -p $(ODIR)
	mkdir -p build
	cp -RTv data build/data/

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o
