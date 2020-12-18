# NoMalloc

*originally named libNanoNF by Qcloud1223*

A lightweight sandbox library. Full functionality to build application which load library and run it with controlled memory region - the library could not touch any memory outside its private heap, and both heap and global storage's location could be customized.

To build from source:

```
$ mkdir build && cd build
$ cmake ..
$ make
```

And `libNoMalloc.a` and example execuables will be built inside `build` directory.

It is recommend to use VS Code with CMake extension for development.

The library provides a set of easy-to-use interfaces in `Include/Box.h`. See the source and comments of `Examples/OneBoxOneFuncOnce.c` for explaination of them. You can also use other low-level interfaces if you like.

**Relation to upstream:** the `master` branch will NOT merge into upstream any more. Instead, any new features that should shared by both repos will be developed in `dev` branch and `dev` will be merged into `master` and (probably) pulled by upstream periodically.