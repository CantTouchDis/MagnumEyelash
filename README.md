# MagnumEyelash

## Compiling the code

To compile the code you first need `corrade` `magnum` and `magnum-plugins`. [Detailed description](https://doc.magnum.graphics/magnum/getting-started.html#getting-started-setup-subproject)

Clone the following three repositories into this folder:
```
git clone https://github.com/mosra/corrade.git
git clone https://github.com/mosra/magnum.git
git clone https://github.com/mosra/magnum-plugins.git
```
Then create a build folder and proceed to build the project as a normal cmake project:
```
mkdir build
cd build
cmake ..
make
```

All the executables will land in the bin folder.
