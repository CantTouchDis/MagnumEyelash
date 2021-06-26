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

## Example
![FirstHair](https://user-images.githubusercontent.com/5838555/122686787-6378ba00-d213-11eb-87c5-3ca4c84c6ef6.png)

## TODO

- add ui to switch uniforms in shaders
- add ui to switch shaders (wireframe, normal, ect.)
- nicer shading
- *maybe* adaptive Geometry shader segment count
