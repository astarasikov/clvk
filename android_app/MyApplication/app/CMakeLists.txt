cmake_minimum_required(VERSION 3.4.1)
set(CMAKE_CXX11_EXTENSION_COMPILE_OPTION -std=gnu++11)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#Sources for our JNI module
add_library(clvk_jni SHARED src/main/jni/jni_part.cpp
            src/main/jni/clinfo/src/clinfo.c
            src/main/jni/intel_samples/BitonicSort/BitonicSort.cpp
            src/main/jni/intel_samples/common/basic.cpp
            src/main/jni/intel_samples/common/cmdparser.cpp
            src/main/jni/intel_samples/common/oclobject.cpp
            src/main/jni/intel_samples/common/utils.cpp
            src/main/jni/intel_samples/common/yuv_utils.cpp
            )

include_directories (
    src/main/jni/opencl/include/
    src/main/jni/intel_samples/common
)

add_library(OpenCL SHARED IMPORTED)
set_target_properties(OpenCL PROPERTIES IMPORTED_LOCATION
       ${CMAKE_SOURCE_DIR}/src/main/jni/opencl/lib/${ANDROID_ABI}/libOpenCL.so)

find_library(log-lib log OpenCL)
 
target_link_libraries(clvk_jni
                    OpenCL
					${log-lib})
