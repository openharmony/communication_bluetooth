CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(map_mse_test CXX C)

SET(TARGET_NAME map_mse_test)
ADD_DEFINITIONS("-coverage")

INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/."
  "${BLUETOOTH_SOURCE_DIR}/external/gtest/include"
  "${BLUETOOTH_SOURCE_DIR}/external/libxml2/include"
  "${BLUETOOTH_SOURCE_DIR}/external/log/include"
  "${BLUETOOTH_SOURCE_DIR}/external/dependence_stub/include"
  "${BLUETOOTH_SOURCE_DIR}/external/digest/src/md5"
  "${BLUETOOTH_SOURCE_DIR}/external/digest/include"  
  "${BLUETOOTH_SOURCE_DIR}/external/database/include"
  "${BLUETOOTH_SOURCE_DIR}/framework/include"
  "${BLUETOOTH_SOURCE_DIR}/service/include"
  "${BLUETOOTH_SOURCE_DIR}/service/tests/common_mock"
	"${BLUETOOTH_SOURCE_DIR}/service/src/base"
	"${BLUETOOTH_SOURCE_DIR}/service/src/util"
  "${BLUETOOTH_SOURCE_DIR}/stack/include"
  "${BLUETOOTH_SOURCE_DIR}/stack"
	"${BLUETOOTH_SOURCE_DIR}/service/src"
	"${BLUETOOTH_SOURCE_DIR}/service/src/obex"
	"${BLUETOOTH_SOURCE_DIR}/service/src/transport"
	"${BLUETOOTH_SOURCE_DIR}/service/src/map_mse"
  "${BLUETOOTH_SOURCE_DIR}/service/tests/map_mse"
  "${BLUETOOTH_SOURCE_DIR}/common"
  "${BLUETOOTH_SOURCE_DIR}/ipc/common"
  "${BLUETOOTH_SOURCE_DIR}/external/ohos_util/include"
)

SET(EXECUTABLE_OUTPUT_PATH ${BLUETOOTH_SOURCE_DIR}/bin/)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/tests/common_mock COMMON_MOCK)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/src/util SRC_UTIL)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/external/digest/src/md5 SRC_MD5)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/src/obex SRC_OBEX)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/tests/obex/sock_mock SOCK_MOCK)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/src/base SRC_BASE)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/src/map_mse SRC_MAP_MSE)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/tests/map_mse TEST_MAP_MSE)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/tests/map_mse/mse_mock MSE_MOCK)

list(APPEND SRC_STACK_TEST ${BLUETOOTH_SOURCE_DIR}/stack/platform/src/packet.c ${BLUETOOTH_SOURCE_DIR}/stack/platform/src/buffer.c ${BLUETOOTH_SOURCE_DIR}/stack/platform/src/allocator.c)
list(APPEND SRC_DIGEST ${BLUETOOTH_SOURCE_DIR}/external/digest/src/message_digest.cpp)
list(APPEND SRC_STUB ${BLUETOOTH_SOURCE_DIR}/external/dependence_stub/passthrough/map_service.cpp)
list(APPEND SRC_COMMON_TYPE ${BLUETOOTH_SOURCE_DIR}/ipc/common/raw_address.cpp)
list(REMOVE_ITEM SRC_UTIL EXCLUDE ${BLUETOOTH_SOURCE_DIR}/service/src/util/dispatcher.cpp)
list(APPEND SRC_UTIL ${BLUETOOTH_SOURCE_DIR}/service/tests/obex/util_mock/dispatcher.cpp)

LINK_DIRECTORIES(${PROJECT_BINARY_DIR}/lib
    "${BLUETOOTH_SOURCE_DIR}/lib/"
    "${BLUETOOTH_SOURCE_DIR}/external/gtest/lib"
)

# Targets greeter_[async_](client|server)
ADD_EXECUTABLE(${TARGET_NAME}
  ${COMMON_MOCK}
  ${TEST_MAP_MSE}
  ${MSE_MOCK}
  ${SRC_MAP_MSE}
  ${SOCK_MOCK}
  ${SRC_STACK_TEST}
  ${SRC_COMMON_TYPE}
  ${SRC_OBEX}
  ${SRC_MD5}
  ${SRC_UTIL}
  ${SRC_BASE}
  ${SRC_DIGEST}
  ${SRC_STUB}
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
  gmock
  gtest
  --coverage
  btdatabase
  utilsecurec_shared
  xml2
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
