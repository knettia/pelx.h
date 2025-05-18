# Overview

`pelx.h` is a mini library for decoding and encoding the PELX file format, a format used for storing images that make use of palettes for rendering.

# Demo

A demo on how to use the library:
```c
#define PELX_with_implementation 1
#include "pelx.h" // or <pelx.h> if used from an include directory

PELX_type(file) pelx_file = NULL;

PELX_type(result) result = PELX_func(decode_pelx)("texture.pelx", &pelx_file); // decode as PELX

PELX_type(palette_entry) entries[] =
{
	{ 0xFF, 0x00, 0x00, 0xFF },
	{ 0x00, 0xFF, 0x00, 0xFF }
};

result = PELX_func(encode_png)("texture_mod.png", pelx_file, 2, entries, 4); // encode as PNG
```

# Example

There also exists a more comprehensive example in the `/example` directory, to test it out:
```sh
cd example
make
./mushrooms.bin
```
