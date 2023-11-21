# !/bin/bash
if [ ! -d "build/arm_linux_4.8" ]; then
  mkdir -p build
  tar xf ./arm_linux_4.8.tar.xz -C build/
fi

cmake -B build . -DENV=$1
cmake --build build