# Prusa-Firmware-MMU-Private

## How to prepare build env and tools
Use tools from the Buddy FW and a separate Atmel AVR GCC 5.4.

Extract the AVR-GCC to some dir, e.g. `/home/user/AVRToolchainMMU/avr8-gnu-toolchain-5.4.0`

```
mkdir .dependencies
cd .dependencies
ln -s ../../STM32Toolchain/clang-format-9.0.0-noext
ln -s ../../STM32Toolchain/cmake-3.15.5
ln -s ../../STM32Toolchain/ninja-1.9.0
ln -s ../../AVRToolchainMMU/avr8-gnu-toolchain-5.4.0
```

## How to build the preliminary project so far:
Please make sure you have your `cmake`, `clang-format` and `ninja` in path

```
mkdir build
cd build
cmake .. -G Ninja -DCMAKE_TOOLCHAIN_FILE=../../cmake/AnyAvrGcc.cmake
ninja
```

Should produce a firmware.hex file
