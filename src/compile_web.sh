#!/bin/sh

export USE_EMCC=1
export USE_WEBGL2=${1:-1}
emmake make

cd build
mv game game.bc
emcc -O2 -s USE_WEBGL2=$USE_WEBGL2 -s USE_GLFW=3 -s USE_FREETYPE=1 -s ALLOW_MEMORY_GROWTH=1 -s EMULATE_FUNCTION_POINTER_CASTS=0 -s WASM=1 --closure 1 --llvm-lto 1 -lopenal -O3 --shell-file data/template.html --preload-file data game.bc -o index.html
cd ..