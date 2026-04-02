# Install script for directory: /opt/nordic/ncs/v3.0.2/zephyr

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/opt/nordic/ncs/toolchains/ef4fc6722e/opt/zephyr-sdk/arm-zephyr-eabi/bin/arm-zephyr-eabi-objdump")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/tydin/Documents/Penn/2025-2026/ESE_3500/final-project-whear/app_core/build/app_core/zephyr/arch/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/tydin/Documents/Penn/2025-2026/ESE_3500/final-project-whear/app_core/build/app_core/zephyr/lib/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/tydin/Documents/Penn/2025-2026/ESE_3500/final-project-whear/app_core/build/app_core/zephyr/soc/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/tydin/Documents/Penn/2025-2026/ESE_3500/final-project-whear/app_core/build/app_core/zephyr/boards/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/tydin/Documents/Penn/2025-2026/ESE_3500/final-project-whear/app_core/build/app_core/zephyr/subsys/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/tydin/Documents/Penn/2025-2026/ESE_3500/final-project-whear/app_core/build/app_core/zephyr/drivers/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/tydin/Documents/Penn/2025-2026/ESE_3500/final-project-whear/app_core/build/app_core/modules/nrf/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/tydin/Documents/Penn/2025-2026/ESE_3500/final-project-whear/app_core/build/app_core/modules/hostap/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/tydin/Documents/Penn/2025-2026/ESE_3500/final-project-whear/app_core/build/app_core/modules/mcuboot/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/tydin/Documents/Penn/2025-2026/ESE_3500/final-project-whear/app_core/build/app_core/modules/mbedtls/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/tydin/Documents/Penn/2025-2026/ESE_3500/final-project-whear/app_core/build/app_core/modules/trusted-firmware-m/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/tydin/Documents/Penn/2025-2026/ESE_3500/final-project-whear/app_core/build/app_core/modules/cjson/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/tydin/Documents/Penn/2025-2026/ESE_3500/final-project-whear/app_core/build/app_core/modules/azure-sdk-for-c/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/tydin/Documents/Penn/2025-2026/ESE_3500/final-project-whear/app_core/build/app_core/modules/cirrus-logic/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/tydin/Documents/Penn/2025-2026/ESE_3500/final-project-whear/app_core/build/app_core/modules/openthread/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/tydin/Documents/Penn/2025-2026/ESE_3500/final-project-whear/app_core/build/app_core/modules/suit-processor/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/tydin/Documents/Penn/2025-2026/ESE_3500/final-project-whear/app_core/build/app_core/modules/memfault-firmware-sdk/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/tydin/Documents/Penn/2025-2026/ESE_3500/final-project-whear/app_core/build/app_core/modules/canopennode/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/tydin/Documents/Penn/2025-2026/ESE_3500/final-project-whear/app_core/build/app_core/modules/chre/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/tydin/Documents/Penn/2025-2026/ESE_3500/final-project-whear/app_core/build/app_core/modules/lz4/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/tydin/Documents/Penn/2025-2026/ESE_3500/final-project-whear/app_core/build/app_core/modules/nanopb/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/tydin/Documents/Penn/2025-2026/ESE_3500/final-project-whear/app_core/build/app_core/modules/zscilib/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/tydin/Documents/Penn/2025-2026/ESE_3500/final-project-whear/app_core/build/app_core/modules/cmsis/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/tydin/Documents/Penn/2025-2026/ESE_3500/final-project-whear/app_core/build/app_core/modules/cmsis-dsp/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/tydin/Documents/Penn/2025-2026/ESE_3500/final-project-whear/app_core/build/app_core/modules/cmsis-nn/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/tydin/Documents/Penn/2025-2026/ESE_3500/final-project-whear/app_core/build/app_core/modules/fatfs/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/tydin/Documents/Penn/2025-2026/ESE_3500/final-project-whear/app_core/build/app_core/modules/hal_nordic/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/tydin/Documents/Penn/2025-2026/ESE_3500/final-project-whear/app_core/build/app_core/modules/hal_st/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/tydin/Documents/Penn/2025-2026/ESE_3500/final-project-whear/app_core/build/app_core/modules/hal_tdk/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/tydin/Documents/Penn/2025-2026/ESE_3500/final-project-whear/app_core/build/app_core/modules/hal_wurthelektronik/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/tydin/Documents/Penn/2025-2026/ESE_3500/final-project-whear/app_core/build/app_core/modules/liblc3/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/tydin/Documents/Penn/2025-2026/ESE_3500/final-project-whear/app_core/build/app_core/modules/libmetal/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/tydin/Documents/Penn/2025-2026/ESE_3500/final-project-whear/app_core/build/app_core/modules/littlefs/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/tydin/Documents/Penn/2025-2026/ESE_3500/final-project-whear/app_core/build/app_core/modules/loramac-node/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/tydin/Documents/Penn/2025-2026/ESE_3500/final-project-whear/app_core/build/app_core/modules/lvgl/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/tydin/Documents/Penn/2025-2026/ESE_3500/final-project-whear/app_core/build/app_core/modules/mipi-sys-t/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/tydin/Documents/Penn/2025-2026/ESE_3500/final-project-whear/app_core/build/app_core/modules/nrf_wifi/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/tydin/Documents/Penn/2025-2026/ESE_3500/final-project-whear/app_core/build/app_core/modules/open-amp/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/tydin/Documents/Penn/2025-2026/ESE_3500/final-project-whear/app_core/build/app_core/modules/percepio/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/tydin/Documents/Penn/2025-2026/ESE_3500/final-project-whear/app_core/build/app_core/modules/picolibc/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/tydin/Documents/Penn/2025-2026/ESE_3500/final-project-whear/app_core/build/app_core/modules/segger/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/tydin/Documents/Penn/2025-2026/ESE_3500/final-project-whear/app_core/build/app_core/modules/tinycrypt/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/tydin/Documents/Penn/2025-2026/ESE_3500/final-project-whear/app_core/build/app_core/modules/uoscore-uedhoc/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/tydin/Documents/Penn/2025-2026/ESE_3500/final-project-whear/app_core/build/app_core/modules/zcbor/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/tydin/Documents/Penn/2025-2026/ESE_3500/final-project-whear/app_core/build/app_core/modules/nrfxlib/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/tydin/Documents/Penn/2025-2026/ESE_3500/final-project-whear/app_core/build/app_core/modules/nrf_hw_models/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/tydin/Documents/Penn/2025-2026/ESE_3500/final-project-whear/app_core/build/app_core/modules/connectedhomeip/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/tydin/Documents/Penn/2025-2026/ESE_3500/final-project-whear/app_core/build/app_core/zephyr/kernel/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/tydin/Documents/Penn/2025-2026/ESE_3500/final-project-whear/app_core/build/app_core/zephyr/cmake/flash/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/tydin/Documents/Penn/2025-2026/ESE_3500/final-project-whear/app_core/build/app_core/zephyr/cmake/usage/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/Users/tydin/Documents/Penn/2025-2026/ESE_3500/final-project-whear/app_core/build/app_core/zephyr/cmake/reports/cmake_install.cmake")
endif()

