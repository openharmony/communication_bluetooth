CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(pbap_test CXX C)

SET(TARGET_NAME pbap_test)
ADD_DEFINITIONS("-coverage")

INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/."
	"${BLUETOOTH_SOURCE_DIR}/external/gtest/include"
  "${BLUETOOTH_SOURCE_DIR}/external/libxml2/include"
	"${BLUETOOTH_SOURCE_DIR}/external/log/include"
	"${BLUETOOTH_SOURCE_DIR}/external/dependence_stub/include"
	"${BLUETOOTH_SOURCE_DIR}/external/dependence_stub/include/stub"
	"${BLUETOOTH_SOURCE_DIR}/external/digest/src/md5"
  "${BLUETOOTH_SOURCE_DIR}/external/digest/include"  
  "${BLUETOOTH_SOURCE_DIR}/external/vcard/include"
  "${BLUETOOTH_SOURCE_DIR}/external/database/include"
  "${BLUETOOTH_SOURCE_DIR}/external/ohos_util/include"    
  "${BLUETOOTH_SOURCE_DIR}/framework/include"
  "${BLUETOOTH_SOURCE_DIR}/service/include"
  "${BLUETOOTH_SOURCE_DIR}/service/tests/pbap/mock"    
  "${BLUETOOTH_SOURCE_DIR}/service/tests/common_mock"
	"${BLUETOOTH_SOURCE_DIR}/service/src/base"
  "${BLUETOOTH_SOURCE_DIR}/service/src/util"
  "${BLUETOOTH_SOURCE_DIR}/service/src/hfp_hf"
  "${BLUETOOTH_SOURCE_DIR}/service/src/hfp_ag"
	"${BLUETOOTH_SOURCE_DIR}/stack/include"
	"${BLUETOOTH_SOURCE_DIR}/stack"
	"${BLUETOOTH_SOURCE_DIR}/service/src"
	"${BLUETOOTH_SOURCE_DIR}/service/src/obex"
	"${BLUETOOTH_SOURCE_DIR}/service/src/transport"
  "${BLUETOOTH_SOURCE_DIR}/service/src/pbap_pse"
	"${BLUETOOTH_SOURCE_DIR}/service/src/pbap_pce"
  "${BLUETOOTH_SOURCE_DIR}/common"
  "${BLUETOOTH_SOURCE_DIR}/ipc/common"    
  "${BLUETOOTH_SOURCE_DIR}/service/tests/obex/sock_mock"    
)

SET(EXECUTABLE_OUTPUT_PATH ${BLUETOOTH_SOURCE_DIR}/bin/)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/src/util SRC_UTIL)
#AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/src/common SRC_COMMON)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/tests/common_mock COMMON_MOCK)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/src/obex SRC_OBEX)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/ipc/common SRC_IPC)
#AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/src/classic SRC_CLASSIC)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/src/base SRC_BASE)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/src/pbap_pse SRC_PBAP_PSE)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/src/pbap_pce SRC_PBAP_PCE)

AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/tests/pbap/mock SRC_PBAP_MOCK)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/tests/obex/sock_mock SRC_SOCK_MOCK)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/tests/pbap SRC_TEST_PBAP)
list(APPEND SRC_STACK ${BLUETOOTH_SOURCE_DIR}/stack/platform/src/packet.c ${BLUETOOTH_SOURCE_DIR}/stack/platform/src/buffer.c ${BLUETOOTH_SOURCE_DIR}/stack/platform/src/allocator.c)
list(REMOVE_ITEM SRC_UTIL EXCLUDE ${BLUETOOTH_SOURCE_DIR}/service/src/util/dispatcher.cpp)
list(APPEND SRC_UTIL ${BLUETOOTH_SOURCE_DIR}/service/tests/obex/util_mock/dispatcher.cpp)


LINK_DIRECTORIES(${PROJECT_BINARY_DIR}/lib
    "${BLUETOOTH_SOURCE_DIR}/lib/"
    "${BLUETOOTH_SOURCE_DIR}/external/gtest/lib"
)

# Targets greeter_[async_](client|server)
ADD_EXECUTABLE(${TARGET_NAME}
  ${COMMON_MOCK}
  ${SRC_SOCK_MOCK}
  ${SRC_PBAP_MOCK}
  ${SRC_BASE}
  ${SRC_OBEX}
  ${SRC_TEST_PBAP}
  ${SRC_UTIL}
  ${SRC_STACK}
  ${SRC_PBAP_PCE}
  ${SRC_PBAP_PSE}
  ${SRC_IPC}
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
  btdigest
  btvcard
  gmock
  gtest
  btdatabase
  utilsecurec_shared
  --coverage
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
