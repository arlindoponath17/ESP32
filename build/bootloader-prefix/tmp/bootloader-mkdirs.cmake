# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/a/Downloads/esp-idf-v5.4.1/components/bootloader/subproject"
  "/home/a/Documentos/workspace_esp32/projeto_TCP_Contador/build/bootloader"
  "/home/a/Documentos/workspace_esp32/projeto_TCP_Contador/build/bootloader-prefix"
  "/home/a/Documentos/workspace_esp32/projeto_TCP_Contador/build/bootloader-prefix/tmp"
  "/home/a/Documentos/workspace_esp32/projeto_TCP_Contador/build/bootloader-prefix/src/bootloader-stamp"
  "/home/a/Documentos/workspace_esp32/projeto_TCP_Contador/build/bootloader-prefix/src"
  "/home/a/Documentos/workspace_esp32/projeto_TCP_Contador/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/a/Documentos/workspace_esp32/projeto_TCP_Contador/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/a/Documentos/workspace_esp32/projeto_TCP_Contador/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
