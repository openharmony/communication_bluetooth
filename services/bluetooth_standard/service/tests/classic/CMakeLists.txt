CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(classic_service_test CXX C)

SET(TARGET_NAME classic_service_test)

INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/."
    "${BLUETOOTH_SOURCE_DIR}/service/tests/common_mock"
    "${BLUETOOTH_SOURCE_DIR}/external/gtest/googletest/include"	
    "${BLUETOOTH_SOURCE_DIR}/external/gtest/googlemock/include"
    "${BLUETOOTH_SOURCE_DIR}/external/log/include"
    "${BLUETOOTH_SOURCE_DIR}/framework/include"
    "${BLUETOOTH_SOURCE_DIR}/service/include"
    "${BLUETOOTH_SOURCE_DIR}/service/tests/classic/mock"
    "${BLUETOOTH_SOURCE_DIR}/service/src/common"
    "${BLUETOOTH_SOURCE_DIR}/service/src/base"
    "${BLUETOOTH_SOURCE_DIR}/service/src/util"
    "${BLUETOOTH_SOURCE_DIR}/service/src/classic"
    "${BLUETOOTH_SOURCE_DIR}/service/src/ble"
    "${BLUETOOTH_SOURCE_DIR}/service/src/a2dp_src"
    "${BLUETOOTH_SOURCE_DIR}/service/src/a2dp_snk"
    "${BLUETOOTH_SOURCE_DIR}/service/src/gatt"
    "${BLUETOOTH_SOURCE_DIR}/service/src"
    "${BLUETOOTH_SOURCE_DIR}/stack/"
    "${BLUETOOTH_SOURCE_DIR}/stack/include/"
    "${BLUETOOTH_SOURCE_DIR}/stack/platform/include"
    "${BLUETOOTH_SOURCE_DIR}/common"
    "${BLUETOOTH_SOURCE_DIR}/ipc/common"
    "${BLUETOOTH_SOURCE_DIR}/external/ohos_util/include"
)

SET(EXECUTABLE_OUTPUT_PATH ${BLUETOOTH_SOURCE_DIR}/bin/)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)


AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/src/classic SRC_CLASSIC)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/tests/classic TEST_CLASSIC)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/tests/classic/mock CLASSIC_MOCK)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/ipc/common SRC_IPC_COMMON)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/tests/common_mock COMMON_MOCK)
list(REMOVE_ITEM COMMON_MOCK EXCLUDE ${BLUETOOTH_SOURCE_DIR}/service/tests/common_mock/adapter_device_config_mock.cpp)

LINK_DIRECTORIES(${PROJECT_BINARY_DIR}/lib
    "${BLUETOOTH_SOURCE_DIR}/lib/"
    "${BLUETOOTH_SOURCE_DIR}/external/gtest/lib/"
)

ADD_DEFINITIONS("--coverage")

ADD_EXECUTABLE(classic_service_test
    ${COMMON_MOCK}
    ${CLASSIC_MOCK}
    ${TEST_CLASSIC}
    ${SRC_CLASSIC}
    ${BLUETOOTH_SOURCE_DIR}/service/src/common/adapter_device_config.cpp
    ${SRC_IPC_COMMON}
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
  gmock
  gtest
  btlog
  btstack
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
