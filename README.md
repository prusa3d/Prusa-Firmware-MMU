# Prusa-Firmware-MMU-Private

## How to prepare build env and tools

run `./utils/bootstrap.py`


`bootstrap.py` will now download all the "missing" dependencies into the `.dependencies` folder:
- clang-format-9.0.0-noext
- cmake-3.22.5
- ninja-1.10.2
- gcc-avr-7.3.0

## How to build the preliminary project so far:
Now the process is the same as in the Buddy Firmware:
```
utils/build.py
```

builds the firmware.hex in build/mmu_release

In case you'd like to build the project directly via cmake you can use an approach like this:
```
mkdir build
cd build
cmake .. -G Ninja -DCMAKE_TOOLCHAIN_FILE=../cmake/AnyAvrGcc.cmake
ninja
```

Should produce a MM-control-01.hex file as well.
