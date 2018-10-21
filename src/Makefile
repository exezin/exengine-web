# Example Makefile

# -- Dirs -- #
CC 			=emcc
CPP     =g++
EDIR    =exengine
BDIR		=build
ODIR 		=obj
IDIR    =inc
_IDIRS  =inc
IDIRS   =$(patsubst %,-I%/,$(_IDIRS))

# -- Flags -- #
FLAGS 	=-lm -lstdc++ -s USE_WEBGL2=$(USE_WEBGL2) -s USE_GLFW=3 -s USE_FREETYPE=1 -s ALLOW_MEMORY_GROWTH=1 -lopenal -I. $(IDIRS) -O3
CFLAGS  =$(FLAGS)
CFLAGS +=-std=c99 -O3
CPPFLAGS=

# -- Files -- #
# engine deps
_EDEPS  =exe_conf.h exe_io.h window.h shader.h mesh.h mathlib.h camera.h \
texture.h stb_image.h iqm.h framebuffer.h pointlight.h exe_list.h scene.h \
model.h dirlight.h skybox.h collision.h entity.h octree.h \
gbuffer.h spotlight.h vertices.h ssao.h engine.h defaults.h input.h \
sound.h cache.h
EDEPS		=$(patsubst %,$(EDIR)/%,$(_EDEPS))

# engine srcs
ENGINEOBJ = window.o mesh.o camera.o texture.o iqm.o \
framebuffer.o pointlight.o scene.o model.o dirlight.o skybox.o \
collision.o entity.o octree.o gbuffer.o spotlight.o \
ssao.o engine.o shader.o defaults.o input.o sound.o cache.o

# usr srcs (add yours here)
_OBJ		=main.o game.o
_OBJ   +=$(LOBJ)
_OBJ   +=$(ENGINEOBJ)
OBJ 		=$(patsubst %,$(ODIR)/%,$(_OBJ))

# user deps
_DEPS		=game.h
DEPS		=$(patsubst %,$(IDIR)/%,$(_DEPS))

all: files game

# user src
$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

# engine srcs
$(ODIR)/%.o: $(EDIR)/%.c $(EDEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

game: $(OBJ)
	$(CC) -o $(BDIR)/$@ $^ $(CFLAGS)
	@echo "**success**"

files:
	mkdir -p $(ODIR)
	mkdir -p $(BDIR)/licence
	cp licence/* $(BDIR)/licence/
	cp -RTv data build/data/
	mv build/data/shaders/gles3/* build/data/shaders/

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o
