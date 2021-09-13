CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(sock_test CXX C)

SET(TARGET_NAME sock_test)

INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/."
  "${BLUETOOTH_SOURCE_DIR}/service/tests/common_mock"
  "${BLUETOOTH_SOURCE_DIR}/external/gtest/googletest/include"	
	"${BLUETOOTH_SOURCE_DIR}/external/gtest/googlemock/include"
  "${BLUETOOTH_SOURCE_DIR}/external/libxml2/include"
	"${BLUETOOTH_SOURCE_DIR}/external/log/include"
  "${BLUETOOTH_SOURCE_DIR}/framework/include"
  "${BLUETOOTH_SOURCE_DIR}/ipc/common"
  "${BLUETOOTH_SOURCE_DIR}/common"
  "${BLUETOOTH_SOURCE_DIR}/service/include"
	"${BLUETOOTH_SOURCE_DIR}/service/src/base"
  "${BLUETOOTH_SOURCE_DIR}/service/src"
	"${BLUETOOTH_SOURCE_DIR}/service/src/util"
	"${BLUETOOTH_SOURCE_DIR}/stack/include"
	"${BLUETOOTH_SOURCE_DIR}/stack"
  "${BLUETOOTH_SOURCE_DIR}/external/ohos_util/include"
  "${BLUETOOTH_SOURCE_DIR}/service/tests/sock/sock_mock"
	"${BLUETOOTH_SOURCE_DIR}/service/src/sock"
	"${BLUETOOTH_SOURCE_DIR}/service/src/transport"
	"${BLUETOOTH_SOURCE_DIR}/service/tests/sock"
)

SET(EXECUTABLE_OUTPUT_PATH ${BLUETOOTH_SOURCE_DIR}/bin/)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/tests/common_mock COMMON_MOCK)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/src/base SRC_BASE)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/ipc/common SRC_IPC)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/src/util SRC_UTIL)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/tests/sock/sock_mock SRC_SOCK_MOCK)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/src/transport SRC_TRAN)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/src/sock SRC_SOCK)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/tests/sock SRC_TEST_SOCK)
list(APPEND SRC_STACK ${BLUETOOTH_SOURCE_DIR}/stack/platform/src/packet.c ${BLUETOOTH_SOURCE_DIR}/stack/platform/src/buffer.c ${BLUETOOTH_SOURCE_DIR}/stack/platform/src/allocator.c)

LINK_DIRECTORIES(${PROJECT_BINARY_DIR}/lib
    "${BLUETOOTH_SOURCE_DIR}/lib/"
    "${BLUETOOTH_SOURCE_DIR}/external/gtest/lib"
)

ADD_DEFINITIONS("--coverage")

# Targets greeter_[async_](client|server)
ADD_EXECUTABLE(${TARGET_NAME}  
  ${COMMON_MOCK}
  ${SRC_BASE} 
  ${SRC_IPC}  
  ${SRC_UTIL} 
  ${SRC_SOCK_MOCK}
  ${SRC_TRAN}
  ${SRC_SOCK}
  ${SRC_TEST_SOCK}
  ${SRC_STACK}
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
  --coverage
  pthread
  gcov
  -fprofile-arcs
  xml2
  btlog
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
