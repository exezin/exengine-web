#!/bin/bash

export USE_EMCC=1
emmake make

mv build/proj build/proj.bc
emcc -O2 -s USE_WEBGL2=1 -s USE_GLFW=3 -s USE_FREETYPE=1 -s ALLOW_MEMORY_GROWTH=1 -lopenal --preload-file data build/proj.bc -o build/index.html
