# --------------------------------------------------------------------------
#
#              //===//   //=====   //===//   //       //   //===//
#             //    //  //        //    //  //       //   //    //
#            //===//   //=====   //===//   //       //   //===<<
#           //   \\         //  //        //       //   //    //
#          //     \\  =====//  //        //=====  //   //===//   Version III
#
# ------------- An Efficient RSerPool Prototype Implementation -------------
#
# Copyright (C) 2002-2019 by Thomas Dreibholz
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Contact: dreibh@iem.uni-due.de

CMAKE_MINIMUM_REQUIRED(VERSION 3.0.2)
PROJECT(rsplib LANGUAGES C CXX)

SET(BUILD_MAJOR "3")
SET(BUILD_MINOR "2")
SET(BUILD_PATCH "5~rc1")
SET(BUILD_VERSION ${BUILD_MAJOR}.${BUILD_MINOR}.${BUILD_PATCH})


#############################################################################
# INSTALLATION_DIRECTORIES
#############################################################################

# See: https://cmake.org/cmake/help/v3.0/module/GNUInstallDirs.html
INCLUDE(GNUInstallDirs)


#############################################################################
# PACKAGING
#############################################################################

SET(CPACK_SOURCE_GENERATOR "TXZ")
SET(CPACK_SOURCE_PACKAGE_FILE_NAME
   "${CMAKE_PROJECT_NAME}-${BUILD_MAJOR}.${BUILD_MINOR}.${BUILD_PATCH}")
SET(CPACK_SOURCE_IGNORE_FILES
   "${PROJECT_SOURCE_DIR}/${CMAKE_PROJECT_NAME}-*;${PROJECT_SOURCE_DIR}/${CMAKE_PROJECT_NAME}_*;/build/;/.git/;/config.log;/CMakeFiles/;CMakeCache.txt$;.cmake$;.tar.gz$;/_CPack_Packages;/Makefile$;~$;/build-deb;/clean-deb;/filter-empty-entries;/make-symbols;/make-ppa;/make-deb;/debian.conf;/make-rpm;/rpm.conf;${CPACK_SOURCE_IGNORE_FILES}")
INCLUDE(CPack)

ADD_CUSTOM_TARGET(dist COMMAND ${CMAKE_MAKE_PROGRAM} clean package_source)


INCLUDE(CheckIncludeFile)
INCLUDE(CheckStructHasMember)


#############################################################################
# OS-DEPENDENT
#############################################################################

IF (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    MESSAGE(STATUS ${CMAKE_SYSTEM_NAME} " supported")
    ADD_DEFINITIONS("-D_DEFAULT_SOURCE -DLINUX")

ELSEIF (${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
    MESSAGE(STATUS ${CMAKE_SYSTEM_NAME} " supported")
    SET(CMAKE_REQUIRED_INCLUDES "/usr/local/include" "/usr/include")
    SET(CMAKE_LIBRARY_PATH "/usr/local/lib")
    INCLUDE_DIRECTORIES("/usr/local/include")
    ADD_DEFINITIONS("-DFreeBSD")

ELSEIF (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    MESSAGE(STATUS ${CMAKE_SYSTEM_NAME} " supported")
    SET(CMAKE_REQUIRED_INCLUDES "/usr/local/include" "/usr/include" "/usr/local/opt/openssl/include")
    SET(CMAKE_LIBRARY_PATH "/usr/local/lib")
    INCLUDE_DIRECTORIES("/usr/local/include" "/usr/local/opt/openssl/include")
    ADD_DEFINITIONS("-DDARWIN")

ELSEIF (${CMAKE_SYSTEM_NAME} MATCHES "NetBSD")
    MESSAGE(STATUS ${CMAKE_SYSTEM_NAME} " supported")
    LIST(APPEND neat_headers neat_bsd.h)
    LIST(APPEND neat_sources neat_bsd.c)
    SET(CMAKE_REQUIRED_INCLUDES "/usr/pkg/include" "/usr/include" "/usr/local/include")
    SET(CMAKE_LIBRARY_PATH "/usr/local/lib")
    INCLUDE_DIRECTORIES("/usr/pkg/include" "/usr/local/include")
    ADD_DEFINITIONS("-DNetBSD")

ELSE()
    MESSAGE(FATAL_ERROR ${CMAKE_SYSTEM_NAME} " not supported (yet?)")

ENDIF()


#############################################################################
# CHECK STRUCT MEMBERS
#############################################################################

CHECK_STRUCT_HAS_MEMBER("struct sockaddr" "sa_len" "sys/types.h;sys/socket.h" HAVE_SA_LEN)
IF (HAVE_SA_LEN)
   MESSAGE(STATUS "HAVE_SA_LEN")
   ADD_DEFINITIONS(-DHAVE_SA_LEN)
ENDIF()

CHECK_STRUCT_HAS_MEMBER("struct sockaddr_in" "sin_len" "sys/types.h;netinet/in.h" HAVE_SIN_LEN)
IF (HAVE_SIN_LEN)
   MESSAGE(STATUS "HAVE_SIN_LEN")
   ADD_DEFINITIONS(-DHAVE_SIN_LEN)
ENDIF()

CHECK_STRUCT_HAS_MEMBER("struct sockaddr_in6" "sin6_len" "sys/types.h;netinet/in.h" HAVE_SIN6_LEN)
IF (HAVE_SIN6_LEN)
   MESSAGE(STATUS "HAVE_SIN6_LEN")
   ADD_DEFINITIONS(-DHAVE_SIN6_LEN)
ENDIF()

CHECK_STRUCT_HAS_MEMBER("struct sockaddr_storage" "ss_len" "sys/types.h;sys/socket.h" HAVE_SS_LEN)
IF (HAVE_SS_LEN)
   MESSAGE(STATUS "HAVE_SS_LEN")
   ADD_DEFINITIONS(-DHAVE_SS_LEN)
ENDIF()


#############################################################################
# CHECK OPTIONS
#############################################################################

# Debug build type as default
IF (NOT CMAKE_BUILD_TYPE)
   MESSAGE(STATUS "No build type selected, using DEBUG")
   SET(CMAKE_BUILD_TYPE "DEBUG")
   # _FORTIFY_SOURCE requires compiling with optimization
   SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O")
ENDIF()

# Maximum logging level
SET(MAX_LOGLEVEL "9" CACHE STRING "Maximum logging level")
ADD_DEFINITIONS(-DMAX_LOGLEVEL=${MAX_LOGLEVEL})

# Registrar statistics
OPTION(ENABLE_REGISTRAR_STATISTICS "Enable Registrar statistics" 1)
IF (ENABLE_REGISTRAR_STATISTICS)
   ADD_DEFINITIONS(-DENABLE_REGISTRAR_STATISTICS)
ENDIF()

# Handlespace Management verification
OPTION(ENABLE_HSMGTVERIFY "Enable handlespace management verification" 0)
IF(ENABLE_HSMGTVERIFY)
   ADD_DEFINITIONS(-DVERIFY)
ENDIF()

# Registrar statistics
OPTION(ENABLE_CSP "Enable CSP" 1)
IF (ENABLE_CSP)
   ADD_DEFINITIONS(-DENABLE_CSP)
ENDIF()

# Qt programs
OPTION(ENABLE_QT "Include Qt programs" 1)
IF (ENABLE_QT)
   SET(CMAKE_AUTOMOC ON)
   FIND_PACKAGE(Qt5 COMPONENTS Core Xml Widgets REQUIRED)
ENDIF()

# Test programs
OPTION(BUILD_TEST_PROGRAMS "Build test programs" 0)

# Handlespace management storage implementation
ADD_DEFINITIONS(-DINCLUDE_SIMPLEREDBLACKTREE -DUSE_SIMPLEREDBLACKTREE)


#############################################################################
# REQUIREMENTS
#############################################################################

# ###### Threads ############################################################
FIND_PACKAGE(Threads REQUIRED)

# ###### Qt #################################################################
OPTION(ENABLE_QT "Include Qt programs" 1)
IF (ENABLE_QT)
   SET(CMAKE_AUTOMOC ON)
   FIND_PACKAGE(Qt5 COMPONENTS Core Xml Widgets REQUIRED)
ENDIF()

# ###### SCTP ###############################################################
OPTION(USE_KERNEL_SCTP "Use Kernel SCTP" 1)
IF (USE_KERNEL_SCTP)
   # ====== Kernel SCTP =====================================================
   CHECK_INCLUDE_FILE(netinet/sctp.h HAVE_NETINET_SCTP_H)
   IF (HAVE_NETINET_SCTP_H)
      SET(SCTP_INCLUDE "netinet/sctp.h")
      ADD_DEFINITIONS(-DHAVE_KERNEL_SCTP -DHAVE_NETINET_SCTP_H)
      IF (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
         FIND_LIBRARY(SCTP_LIB sctp REQUIRED)
      ELSE()
         FIND_LIBRARY(SCTP_LIB sctp)
      ENDIF()
      IF (SCTP_LIB)
         SET(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} ${SCTP_LIB})
      ELSE()
         SET(SCTP_LIB "")
      ENDIF()
   ENDIF()

ELSE()
   # ====== SCTPLIB User-Space SCTP =========================================
   CHECK_INCLUDE_FILE(sctp.h HAVE_SCTPLIB_SCTP_H)
   FIND_LIBRARY(SCTPLIB_LIB sctplib)
   IF (NOT HAVE_SCTPLIB_SCTP_H OR NOT SCTPLIB_LIB)
      MESSAGE(FATAL_ERROR "sctplib not found - libsctplib installed?")
   ENDIF()
   MESSAGE(STATUS "sctplib found: ${SCTPLIB_LIB}")

   CHECK_INCLUDE_FILE(sctp.h HAVE_SCTPSOCKET_SCTP_H)
   FIND_LIBRARY(SCTPSOCKET_LIB sctpsocket)
   IF (NOT HAVE_SCTPSOCKET_SCTP_H OR NOT SCTPSOCKET_LIB)
      MESSAGE(FATAL_ERROR "sctpsocket not found - libsctpsocket installed?")
   ENDIF()
   MESSAGE(STATUS "sctpsocket found: ${SCTPSOCKET_LIB}")

   FIND_PACKAGE(PkgConfig)
   PKG_CHECK_MODULES(GLIB glib-2.0)
   IF (NOT ${GLIB_LIBS})
      MESSAGE(FATAL_ERROR "glib not found - glib-2.0 installed?")
   ENDIF()
   MESSAGE(STATUS "glib found: ${GLIB_LIBRARIES}")

   SET(SCTP_LIB ${SCTPSOCKET_LIB} ${SCTPLIB_LIB} ${GLIB_LIBRARIES})
   SET(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} ${SCTPSOCKET_LIB} ${SCTPLIB_LIB} ${GLIB_LIBRARIES})
ENDIF()


# ====== Functions ==========================================================
INCLUDE(CheckFunctionExists)

check_function_exists(sctp_send HAVE_SCTP_SEND)
IF (HAVE_SCTP_SEND)
   ADD_DEFINITIONS(-DHAVE_SCTP_SEND)
ENDIF()

check_function_exists(sctp_sendx HAVE_SCTP_SENDX)
IF (HAVE_SCTP_SENDX)
   ADD_DEFINITIONS(-DHAVE_SCTP_SENDX)
ENDIF()

check_function_exists(sctp_sendmsg  HAVE_SCTP_SENDMSG)
IF (HAVE_SCTP_SENDMSG)
   ADD_DEFINITIONS(-DHAVE_SCTP_SENDMSG)
ENDIF()

check_function_exists(sctp_connectx HAVE_SCTP_CONNECTX)
IF (HAVE_SCTP_CONNECTX)
   ADD_DEFINITIONS(-DHAVE_SCTP_CONNECTX)
ENDIF()


# ###### BZip2 ##############################################################
FIND_PACKAGE(BZip2 REQUIRED)


# ###### IEEE Floating-Point ################################################
INCLUDE(CheckCSourceRuns)
check_c_source_runs("
#include <stdio.h>

union DoubleIntUnion
{
   double             Double;
   unsigned long long Integer;
};

int main (int argc, char** argv)
{
   union DoubleIntUnion value;

   value.Double = 0.0;
   if(value.Integer != 0ULL) {
      return(1);
   }
   value.Double = 1000.5;
   if(value.Integer != 0x408f440000000000ULL) {
      return(1);
   }
   value.Double = -1000.5;
   if(value.Integer != 0xc08f440000000000ULL) {
      return(1);
   }

   return(0);
}"
USES_IEEEFP_FP)

IF (USES_IEEEFP_FP)
   ADD_DEFINITIONS(-DHAVE_IEEE_FP)
ENDIF()


#############################################################################
# COMPILER FLAGS
#############################################################################

SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_CXX_EXTENSIONS OFF)

SET(CMAKE_C_STANDARD 11)
SET(CMAKE_C_STANDARD_REQUIRED ON)
SET(CMAKE_C_EXTENSIONS OFF)

SET(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -Wall")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")


#############################################################################
# SOURCE DIRECTORIES
#############################################################################

ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(docs)
