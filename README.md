# Blackhole

A multi-purpose visual editor for the Galaxy games, aiming for modularity, cross-platform support, and maintainable code.

## Building from source

Because Blackhole is in early stages of development, no binaries are provided. You may follow the instructions below to build the source code and help with development. :)

```console
git clone --recursive https://github.com/RealTheSunCat/Blackhole
```
Make sure to clone Blackhole **recursively**, or it will not build!

Install the Qt5 libraries with your preferred package manager.
You will also need CMake installed.

On Arch Linux, the packages can be installed via the following command:
```console
sudo pacman -S qt5 cmake
```

Finally, you can run CMake to create project files. Here is an example of an out-of-tree build using GNU Makefiles:
```console
mkdir build && cd build
cmake ..
make
./blackhole
```
