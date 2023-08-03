# Zippy
A simple folder zipper using libzip. Zippy is a header only library; to use it, please include `include/Zippy.hpp`

## Requirements

- Please ensure libzip is installed on your system so that cmake can find it.
- Please ensure that your are using c++17 at the minimum.

## Compilation

To compile the tests 

```sh
mkdir build
cd build
cmake ..
make

./Zippy-Tests
```