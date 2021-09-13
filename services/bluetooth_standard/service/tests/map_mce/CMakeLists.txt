CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(map_mce_test CXX C)

SET(TARGET_NAME map_mce_test)

INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/."
  "${BLUETOOTH_SOURCE_DIR}/external/gtest/googletest/include"	
	"${BLUETOOTH_SOURCE_DIR}/external/gtest/googlemock/include"
  "${BLUETOOTH_SOURCE_DIR}/external/libxml2/include"
  "${BLUETOOTH_SOURCE_DIR}/external/md5"
	"${BLUETOOTH_SOURCE_DIR}/external/dependence_stub/include"
	"${BLUETOOTH_SOURCE_DIR}/external/dependence_stub/include/stub"
	"${BLUETOOTH_SOURCE_DIR}/external/digest/include"
	"${BLUETOOTH_SOURCE_DIR}/external/log/include"
  "${BLUETOOTH_SOURCE_DIR}/framework/include"
  "${BLUETOOTH_SOURCE_DIR}/service/include"
	"${BLUETOOTH_SOURCE_DIR}/service/src/common"
	"${BLUETOOTH_SOURCE_DIR}/service/src/base"
	"${BLUETOOTH_SOURCE_DIR}/service/src/util"
	"${BLUETOOTH_SOURCE_DIR}/stack/include"
	"${BLUETOOTH_SOURCE_DIR}/stack"
	"${BLUETOOTH_SOURCE_DIR}/service/src"
	"${BLUETOOTH_SOURCE_DIR}/service/src/obex"
	"${BLUETOOTH_SOURCE_DIR}/service/src/transport"
	"${BLUETOOTH_SOURCE_DIR}/service/src/map_mce"
  "${BLUETOOTH_SOURCE_DIR}/service/src/map_mse"
	"${BLUETOOTH_SOURCE_DIR}/service/tests/map_mce"
	"${BLUETOOTH_SOURCE_DIR}/service/tests/common_mock"
	"${BLUETOOTH_SOURCE_DIR}/common"
  "${BLUETOOTH_SOURCE_DIR}/ipc/common"
  "${BLUETOOTH_SOURCE_DIR}/external/ohos_util/include"
)

SET(EXECUTABLE_OUTPUT_PATH ${BLUETOOTH_SOURCE_DIR}/bin/)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/external/digest/src/md5 SRC_MD5)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/src/util SRC_UTIL)
#AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/src/obex SRC_OBEX)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/src/base SRC_BASE)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/tests/map_mce/mce_mock SRC_MCE_MOCK)
#AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/src/map_mse SRC_MAP_MSE)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/tests/map_mce/obex_mock SRC_OBEX_MOCK)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/src/map_mce SRC_MAP_MCE)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/tests/common_mock SRC_COMM_MOCK)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/tests/map_mce SRC_TEST_MAP_MCE)
list(APPEND SRC_STACK ${BLUETOOTH_SOURCE_DIR}/stack/platform/src/packet.c ${BLUETOOTH_SOURCE_DIR}/stack/platform/src/buffer.c ${BLUETOOTH_SOURCE_DIR}/stack/platform/src/allocator.c)
list(APPEND SRC_DIGEST ${BLUETOOTH_SOURCE_DIR}/external/digest/src/message_digest.cpp)
list(APPEND SRC_OBEX_FILE ${BLUETOOTH_SOURCE_DIR}/service/src/obex/obex_headers.cpp ${BLUETOOTH_SOURCE_DIR}/service/src/obex/obex_transport.cpp ${BLUETOOTH_SOURCE_DIR}/service/src/obex/obex_utils.cpp ${BLUETOOTH_SOURCE_DIR}/service/src/obex/obex_body.cpp)
list(APPEND SRC_COMMON_TYPE ${BLUETOOTH_SOURCE_DIR}/ipc/common/raw_address.cpp)
list(REMOVE_ITEM SRC_UTIL EXCLUDE ${BLUETOOTH_SOURCE_DIR}/service/src/util/dispatcher.cpp)
list(REMOVE_ITEM SRC_COMM_MOCK EXCLUDE ${BLUETOOTH_SOURCE_DIR}/service/tests/common_mock/power_manager_mock.cpp)
list(APPEND SRC_UTIL ${BLUETOOTH_SOURCE_DIR}/service/tests/obex/util_mock/dispatcher.cpp)

LINK_DIRECTORIES(${PROJECT_BINARY_DIR}/lib
    "${BLUETOOTH_SOURCE_DIR}/lib/"
    "${BLUETOOTH_SOURCE_DIR}/external/gtest/lib"
)

ADD_DEFINITIONS("--coverage")

# Targets greeter_[async_](client|server)
ADD_EXECUTABLE(${TARGET_NAME}
  ${SRC_COMMON_TYPE}
  ${SRC_COMM_MOCK}
  ${SRC_MCE_MOCK}
  ${SRC_BASE}
  ${SRC_DIGEST}
  ${SRC_MD5}
  ${SRC_OBEX_MOCK}
  ${SRC_TEST_MAP_MCE}
  ${SRC_UTIL}
  ${SRC_STACK}
  ${SRC_OBEX_FILE}
  ${SRC_MAP_MCE}
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
