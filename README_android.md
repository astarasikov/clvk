# Building CLVK for Android

First, perform a regular build procedure for the Host, outlined
in README.md.
This is necessary to check out the dependencies.
Additionally, a host build of LLVM is necessary to cross-compile LLVM
and the easiest way to get a compatible one is to first perform the build
for the host.

```
mkdir build_android
cd build_android
../compile_android.sh
make -j32
```

Now, you have "libOpenCL.so" which you can copy to your Android application
project for Android Studio/gradle.

```
cp libOpenCL.so ../android_app/MyApplication/app/src/main/jni/opencl/lib/arm64-v8a/libOpenCL.so
```

# Android App
Build the app (in the `android_app`) directory with Android Studio
or use gradle on the command line and install the APK.

After that, we need to push the "clspv" compiler to the device.
We can only push it to the temporary folder via "adb".
```
adb push clspv /data/local/tmp/
adb shell chmod +x /data/local/tmp/clspv
adb shell
```

Now, in the ADB shell, we can run the following command to copy the compiler
to the application directory.
```
run-as io.github.astarasikov.clvkandroid.clvktests cp /data/local/tmp/clspv .
```


# OpenCL sources for the Android App
We also have to push the OpenCL source file for the BitonicSort example in the same way.
```
adb push android_app/MyApplication/app/src/main/jni/intel_samples/BitonicSort/BitonicSort.cl /data/local/tmp/
adb shell run-as io.github.astarasikov.clvkandroid.clvktests cp /data/local/tmp/BitonicSort.cl .
```

# Third-party code
## Bitonic Sort code from Intel OpenCL samples.
* https://software.intel.com/en-us/opencl-sdk/training
* https://software.intel.com/sites/default/files/managed/41/64/intel_ocl_samples.zip

## clinfo.c from clinfo project (clinfo by Giuseppe Bilotta) (CC0 license).
Sources obtained from the following mirror:
* https://github.com/Oblomov/clinfo/blob/master/src/clinfo.c
