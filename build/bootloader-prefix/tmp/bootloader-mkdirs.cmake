# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/vjo/esp/esp-idf/components/bootloader/subproject"
  "/home/vjo/LapTimer2.0/build/bootloader"
  "/home/vjo/LapTimer2.0/build/bootloader-prefix"
  "/home/vjo/LapTimer2.0/build/bootloader-prefix/tmp"
  "/home/vjo/LapTimer2.0/build/bootloader-prefix/src/bootloader-stamp"
  "/home/vjo/LapTimer2.0/build/bootloader-prefix/src"
  "/home/vjo/LapTimer2.0/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/vjo/LapTimer2.0/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/vjo/LapTimer2.0/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
