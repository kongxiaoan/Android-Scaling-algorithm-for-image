# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.10.2)
#cmake_minimum_required(VERSION 3.22.1)


project("image")
add_library(
        image
        SHARED
        src/main/cpp/native-lib.cpp)
find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

find_library( # Sets the name of the path variable.
        jnigraphics-lib
        jnigraphics)


set(libs "${CMAKE_CURRENT_SOURCE_DIR}/src/main/jnilibs")
message(test-------- ${CMAKE_CURRENT_SOURCE_DIR})
#导入libopencv_java4库文件
add_library(libopencv
        SHARED
        IMPORTED)
set_target_properties(
        libopencv
        PROPERTIES IMPORTED_LOCATION
        "${libs}/${ANDROID_ABI}/libopencv_java4.so")
#/Users/mtkj/AndroidStudioProjects/AndroidScalingalgorithmforimage/app/src/main/jniLibs/arm64-v8a
#/Users/mtkj/AndroidStudioProjects/AndroidScalingalgorithmforimage/app/src/main/cpp/src/main/jnilibs/arm64-v8a/
#导入头文件
include_directories(${CMAKE_SOURCE_DIR}/src/main/cpp/include)
find_library(android-lib android )

target_link_libraries( # Specifies the target library.
        image
        libopencv
        ${jnigraphics-lib}
        # Links the target library to the log library
        # included in the NDK.
        ${log-lib}
        ${android-lib}
        )