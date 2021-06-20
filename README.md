# MagnumEyelash


## Dependencies
Requirements:
- corrade
- magnum + SDL2

To compile the code you first need `corrade` `magnum` and `magnum-plugins`. [Detailed description](https://doc.magnum.graphics/magnum/getting-started.html#getting-started-setup-subproject)

Clone the following three repositories into this folder:
```
git clone https://github.com/mosra/corrade.git
git clone https://github.com/mosra/magnum.git
git clone https://github.com/mosra/magnum-plugins.git
```

## Compiling the code

Create a build folder and proceed to build the project as a normal cmake project:
```
mkdir -p build && cd build
cmake ..
cmake --build .
```


## Usage

W,S,A,D to fly around the scene and mouse left click + drag to rotate.
