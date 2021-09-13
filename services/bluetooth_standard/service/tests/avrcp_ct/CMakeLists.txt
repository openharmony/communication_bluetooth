CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(avrcp_ct_test CXX C)

IF(NOT MSVC) 
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
  IF(APPLE)
    SET(START_WHOLE_ARCHIVE -Wl, -all_load)
  ELSE(LINUX)
    SET(START_WHOLE_ARCHIVE -Wl,--whole-archive)
    SET(END_WHOLE_ARCHIVE -Wl,--no-whole-archive)
  ENDIF(APPLE)
ELSE()
  ADD_DEFINITIONS(-D_WIN32_WINNT=0x600)
ENDIF()

SET(TARGET_NAME avrcp_ct_test)
SET (CMAKE_C_COMPILER gcc)
SET (CMAKE_CXX_COMPILER g++)

INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/."
  "${BLUETOOTH_SOURCE_DIR}/service/tests/common_mock"
  "${BLUETOOTH_SOURCE_DIR}/external/gtest/googletest/include"	
	"${BLUETOOTH_SOURCE_DIR}/external/gtest/googlemock/include"
  "${BLUETOOTH_SOURCE_DIR}/external/libxml2/include"
  "${BLUETOOTH_SOURCE_DIR}/external/ohos_util/include"
	"${BLUETOOTH_SOURCE_DIR}/external/log/include"
  "${BLUETOOTH_SOURCE_DIR}/framework/include"
	"${BLUETOOTH_SOURCE_DIR}/service/src/base"
	"${BLUETOOTH_SOURCE_DIR}/service/src/util"
	"${BLUETOOTH_SOURCE_DIR}/stack/include"
  "${BLUETOOTH_SOURCE_DIR}/stack"
	"${BLUETOOTH_SOURCE_DIR}/service/src/obex"
	"${BLUETOOTH_SOURCE_DIR}/service/src/sock"
	"${BLUETOOTH_SOURCE_DIR}/service/src/avrcp_ct"
  "${BLUETOOTH_SOURCE_DIR}/service/src/avrcp_tg"
  "${BLUETOOTH_SOURCE_DIR}/service/tests/avrcp_ct/mock"
	"${BLUETOOTH_SOURCE_DIR}/service/tests/avrcp_ct"
	"${BLUETOOTH_SOURCE_DIR}/ipc/common"
	"${BLUETOOTH_SOURCE_DIR}/service/include"
	"${BLUETOOTH_SOURCE_DIR}/common"
)


SET(EXECUTABLE_OUTPUT_PATH ${BLUETOOTH_SOURCE_DIR}/bin/)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/ipc/common SRC_IPC)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/src/util SRC_UTIL)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/src/base SRC_BASE)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/tests/common_mock COMMON_MOCK)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/tests/avrcp_ct/mock SRC_MOCK)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/src/avrcp_ct SRC_AVRCP_CT)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/tests/avrcp_ct SRC_TEST_AVRCP_CT)


list(APPEND SRC_STACK 
  ${BLUETOOTH_SOURCE_DIR}/stack/platform/src/packet.c 
  ${BLUETOOTH_SOURCE_DIR}/stack/platform/src/buffer.c 
  ${BLUETOOTH_SOURCE_DIR}/stack/platform/src/allocator.c)

list(APPEND SRC_AVRCP_TG 
  ${BLUETOOTH_SOURCE_DIR}/service/src/avrcp_tg/avrcp_tg_packet.cpp
  ${BLUETOOTH_SOURCE_DIR}/service/src/avrcp_tg/avrcp_tg_browse.cpp
  ${BLUETOOTH_SOURCE_DIR}/service/src/avrcp_tg/avrcp_tg_vendor.cpp
  ${BLUETOOTH_SOURCE_DIR}/service/src/avrcp_tg/avrcp_tg_pass_through.cpp
  ${BLUETOOTH_SOURCE_DIR}/service/src/avrcp_tg/avrcp_tg_unit_info.cpp
  ${BLUETOOTH_SOURCE_DIR}/service/src/avrcp_tg/avrcp_tg_sub_unit_info.cpp
  ${BLUETOOTH_SOURCE_DIR}/service/src/avrcp_tg/avrcp_tg_notification.cpp)

LINK_DIRECTORIES(${PROJECT_BINARY_DIR}/lib
    "${BLUETOOTH_SOURCE_DIR}/lib/"
    "${BLUETOOTH_SOURCE_DIR}/external/gtest/lib"
)

ADD_DEFINITIONS("--coverage")

# Targets greeter_[async_](client|server)
ADD_EXECUTABLE(avrcp_ct_test
  ${SRC_UTIL}
  ${SRC_BASE}
  ${SRC_STACK}
  ${SRC_IPC}
  ${COMMON_MOCK}
  ${SRC_MOCK}
  ${SRC_AVRCP_CT}
  ${SRC_AVRCP_TG}
  ${SRC_TEST_AVRCP_CT}
)

IF(APPLE)
TARGET_LINK_LIBRARIES(${TARGET_NAME}
  clang_rt.profile_osx
  -lpthread
  gmock
  gtest)
ELSE(LINUX)
ADD_DEPENDENCIES(${TARGET_NAME} BluetoothGtest)
TARGET_LINK_LIBRARIES(${TARGET_NAME}
  stdc++
  pthread
  gcov
  -fprofile-arcs
  xml2
  btlog
  --coverage
  gmock
  gtest
  utilsecurec_shared
  )
ENDIF(APPLE)

#display status message for important variables
MESSAGE(STATUS "-------------------------------------------------")
MESSAGE(STATUS "PROJECT_NAME = ${PROJECT_NAME}")
MESSAGE(STATUS "CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")
MESSAGE(STATUS "CMAKE_SYSTEM_NAME = ${CMAKE_SYSTEM_NAME}")
MESSAGE(STATUS "PROJECT_SOURCE_DIR = ${PROJECT_SOURCE_DIR}")
MESSAGE(STATUS "PROJECT_BINARY_DIR = ${PROJECT_BINARY_DIR}")
MESSAGE(STATUS "EXECUTABLE_OUTPUT_PATH = ${EXECUTABLE_OUTPUT_PATH}")
MESSAGE(STATUS "LIBRARY_OUTPUT_PATH = ${LIBRARY_OUTPUT_PATH}")
MESSAGE(STATUS "------------------------------------------------")
MESSAGE(STATUS)
