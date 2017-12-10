# This is a HTML5/WebGL port of [exengine](https://github.com/exezin/exengine-testing) using emscripten

### What exactly is exengine?
Began as an experiment, evolved into something feasible for actually making games.  exengine takes a slightly
different approach than other libraries and engines do, in that its a code-base you include directly into yours.  Rather than using it as a static/shared library.

This approach allows easy and direct access to the engine back-end should you want to make modifications to suit your specific needs.

The code-base is almost entirely C99, with an exception for [imgui](https://github.com/ocornut/imgui) support.

### What differs in this web port?
Because this is a web port using emscripten, the GL code specifically targets ~GLES3, which emscripten maps directly to WebGL2.  While this is primarily a web port of the engine, it can compile to non-web targets with no code changes, just run make!

This uses a forward renderer, unlike the primary engine which uses deferred rendering.

Due to this using ~GLES3 code it will remain in a different codebase to the primary engine and thus will not have all the same features, this port aims to be light and fast.

### What are the features?
* *Simple* and small
* C99 compliant
* IQM model loading
* 3D model animation
* Scene manager
* Polygon soup collision detection
* Smooth collision response
* Various cameras
* View models
* *Most* of the primary engine features

### Depends
* A C99 compiler (gcc, clang etc)
* GLFW3
* GLEW
* Emscripten (If you wish to target webgl)

#### Documentation
Docs will be available once the codebase is stable and *complete*.  For now refer to the engine headers should you want to experiment with this.

#### Using & Compiling
Compiling for web requires that you have emscripten installed and ready to use, simply run the ./compile_web.sh script to do so.

You can run make, which will compile to a binary file for your system, rather than a web release.

### Gallery
![scrot](https://i.imgur.com/iAKEHpT.png)
![scrot](https://i.imgur.com/fsUDSdJ.png)
