##### Texture Atlas
A simple texture runtime atlas partitioning tool made to be embedded in other projects, with very little in terms of dependencies and API agnostic in nature.

### Building (library):
* Download [src/texture_atlas.c](src/texture_atlas.c) and [src/texture_atlas.h](src/texture_atlas.h).
* Integrate them into your project as needed.
* Done.

### Building (example):
* Install both [SDL2](https://www.libsdl.org/download-2.0.php), [SDL2_image](https://www.libsdl.org/projects/SDL_image/) libraries and headers.
* Optional: Set SDL2 install path with `-DDSDL2_PATH=<path>`
* Optional: Set SDL2_image install path with `-DSDL2_IMAGE_PATH=<path>`
* Either install [assimp](http://assimp.org/) library and headers or set `-DWITH_BUILTIN_ASSIMP=ON` when configuring with cmake.
* Clone the git repository, and into it's folder:
```bash
$ mkdir build && cd build
$ cmake ..
$ make
```

#### License:
This is free software. The source files in this repository are released under the [Modified BSD License](LICENSE.md), see the license file for more information.