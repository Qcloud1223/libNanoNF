# libNanoNF
A library to load an ELF into a specific location

## Intro
```c
void *NFopen(const char* file, int mode, void *addr)
```
This function loads an ELF specificed by filename **file** into designated address **addr**, and returns a handle for future use.

```c
void *NFsym(void *l, const char *s)
```
This function takes a handle opened by ```NFopen``` and finds the location of the symbol named **s**.

## Install
```
make
```
And link with ```-lNanoNF```.