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

```c
uint64_t NFusage_worker(const char *name, int mode)
```
This function calculate the space a shared library specified by ``name `` takes up.(already page-aligned)
Note that before ``NFopen``, you must call this function for it resolve the dependencies.
Feel free to place your private heap somewhere next to it.

## Install
```bash
make
cd sample
make lib
make
./ori-malloc
# open another terminal
cat /proc/$(pgrep ori-malloc)/maps
```
This is the simplest example you can run and check if the shared library and their dependencies are loaded compactly, 
and in the libc practice are not.