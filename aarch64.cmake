set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_CROSSCOMPILING TRUE)

set(CMAKE_SYSROOT /mnt/sysroot)
set(CMAKE_FIND_ROOT_PATH ${CMAKE_SYSROOT}/usr/lib/aarch64-linux-gnu)
set(CMAKE_INCLUDE_PATH ${CMAKE_SYSROOT}/usr/include/aarch64-linux-gnu)
set(CMAKE_LIBRARY_PATH ${CMAKE_SYSROOT}/usr/lib/aarch64-linux-gnu)
set(CMAKE_PROGRAM_PATH ${CMAKE_SYSROOT}/usr/bin/aarch64-linux-gnu)

set(CMAKE_PREFIX_PATH ${CMAKE_SYSROOT}/usr/local/qt5/lib/cmake/Qt5)

#set(CMAKE_C_COMPILER /usr/bin/arm-linux-gnueabihf-gcc)
#set(CMAKE_CXX_COMPILER /usr/bin/arm-linux-gnueabihf-g++)

set(CMAKE_C_COMPILER /opt/qt5jxavier/gcc-linaro-7.3.1-2018.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER /opt/qt5jxavier/gcc-linaro-7.3.1-2018.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-g++)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

#set(PKG_CONFIG_EXECUTABLE "/usr/bin/aarch64-linux-gnueabi-pkg-config" CACHE FILEPATH "pkg-config executable")
