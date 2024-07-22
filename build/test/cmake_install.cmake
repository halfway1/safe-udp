# Install script for directory: /work/test

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
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
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

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}/work/build/bin/server" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/work/build/bin/server")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}/work/build/bin/server"
         RPATH "/work/build/lib")
  endif()
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/work/build/bin/server")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/work/build/bin" TYPE EXECUTABLE FILES "/work/build/test/server")
  if(EXISTS "$ENV{DESTDIR}/work/build/bin/server" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/work/build/bin/server")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}/work/build/bin/server"
         OLD_RPATH "/work/build/udp_transport:"
         NEW_RPATH "/work/build/lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/work/build/bin/server")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}/work/build/bin/client" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/work/build/bin/client")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}/work/build/bin/client"
         RPATH "/work/build/lib")
  endif()
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/work/build/bin/client")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/work/build/bin" TYPE EXECUTABLE FILES "/work/build/test/client")
  if(EXISTS "$ENV{DESTDIR}/work/build/bin/client" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/work/build/bin/client")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}/work/build/bin/client"
         OLD_RPATH "/work/build/udp_transport:"
         NEW_RPATH "/work/build/lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/work/build/bin/client")
    endif()
  endif()
endif()
