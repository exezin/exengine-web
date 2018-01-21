# -- Dirs -- #
ifneq ($(USE_EMCC),1)
CC    =gcc
FLAGS =-g -lm -lstdc++ -lGL -lGLEW -lglfw -lfreetype -lopenal -I/usr/include/freetype2
else
CC    =emcc
FLAGS =-s USE_WEBGL2=$(USE_WEBGL2) -s USE_GLFW=3 -s USE_FREETYPE=1 -s ALLOW_MEMORY_GROWTH=1 -lopenal
endif

EDIR  =exengine
BDIR  =build
ODIR 	=obj
CFLAGS  =$(FLAGS)

# -- Files -- #
_EDEPS =camera.h io.h mesh.h scene.h text.h canvas.h collision.h entity.h iqm.h list.h math.h \
model.h octree.h pointlight.h shader.h sound.h stb_image.h text.h texture.h vertices.h window.h
EDEPS  =$(patsubst %,$(EDIR)/%,$(_EDEPS))

# user deps
DEPS		=

# all srcs
_OBJ		=main.o shader.o window.o camera.o texture.o mesh.o model.o iqm.o scene.o text.o sound.o octree.o entity.o collision.o pointlight.o canvas.o
OBJ 		=$(patsubst %,$(ODIR)/%,$(_OBJ))

all: files proj

# usr srcs
$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

# engine srcs
$(ODIR)/%.o: $(EDIR)/%.c $(EDEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

proj: $(OBJ)
	$(CC) -o $(BDIR)/$@ $^ $(CFLAGS)
	@echo "**success**"

files:
ifeq ($(USE_EMCC), 1)
	@echo "using emcc"
	@echo "using webgl" $(USE_WEBGL2)
endif
	mkdir -p $(ODIR)
	mkdir -p build
	cp -RTv data build/data/
ifeq ($(USE_WEBGL2), 0)
	@echo "using gles2 shaders"
	cp -RTv shaders/gles2/ build/data/shaders/
else
	@echo "using gles3 shaders"
	cp -RTv shaders/ build/data/shaders/
endif

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o
