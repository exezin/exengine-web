#!/bin/bash

export USE_EMCC=1
export USE_WEBGL2=${1:-1}
emmake make

# cd required here because emscripten doesnt want to 
# work with data in its immediate dir >_>
cd build
mv proj proj.bc
emcc -O2 -s USE_WEBGL2=$USE_WEBGL2 -s USE_GLFW=3 -s USE_FREETYPE=1 -s ALLOW_MEMORY_GROWTH=1 -lopenal --shell-file data/template.html --preload-file data proj.bc -o index.html
cd ..