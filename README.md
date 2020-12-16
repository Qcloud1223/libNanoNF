# libNanoNF

*or NoMalloc suggested by sgdXBC*

A lightweight sandbox library. Full functionality to build application which load library and run it with controlled memory region - the library could not touch any memory outside its private heap, and both heap and global storage's location could be customized.

See examples for further usage.

To build from source:

```
$ mkdir build && cd build
$ cmake ..
$ make
```

And `libNoMalloc.a` and example execuable `ori-malloc` will be built inside `build` directory.

It is recommend to use VS Code with CMake extension for development.
