#!/bin/bash

# https://gist.github.com/syoyo/9acc46554723db14d3a5

export ANDROID_NDK_HOME=/home/alexander/Documents/workspace/bin/android/ndk-bundle

cmake .. \
	-DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake \
	-DANDROID_ABI=arm64-v8a \
	-DANDROID_PLATFORM=android-27 \
	-DANDROID_ALLOW_UNDEFINED_SYMBOLS=On \
	-DLLVM_HOST_TRIPLE=aarch64-unknown-linux-android \
	-DCROSS_TOOLCHAIN_FLAGS_NATIVE='-DCMAKE_C_COMPILER=cc;-DCMAKE_CXX_COMPILER=c++'
