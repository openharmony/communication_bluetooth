CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(hfp_hf_test CXX C)

SET(TARGET_NAME hfp_hf_test)

INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/."
    "${BLUETOOTH_SOURCE_DIR}/common"
    "${BLUETOOTH_SOURCE_DIR}/external/gtest/googletest/include"	
    "${BLUETOOTH_SOURCE_DIR}/external/gtest/googlemock/include"
    "${BLUETOOTH_SOURCE_DIR}/external/libxml2/include"
    "${BLUETOOTH_SOURCE_DIR}/external/log/include"
    "${BLUETOOTH_SOURCE_DIR}/external/dependence_stub/include"
    "${BLUETOOTH_SOURCE_DIR}/external/ohos_util/include"
    "${BLUETOOTH_SOURCE_DIR}/framework/include"
    "${BLUETOOTH_SOURCE_DIR}/ipc/common"
    "${BLUETOOTH_SOURCE_DIR}/service/include"
    "${BLUETOOTH_SOURCE_DIR}/service/src"
    "${BLUETOOTH_SOURCE_DIR}/service/tests/common_mock"
    "${BLUETOOTH_SOURCE_DIR}/service/src/base"
    "${BLUETOOTH_SOURCE_DIR}/service/src/util"
    "${BLUETOOTH_SOURCE_DIR}/service/src/hfp_hf"
    "${BLUETOOTH_SOURCE_DIR}/service/tests/hfp_hf"
    "${BLUETOOTH_SOURCE_DIR}/stack/include"
    "${BLUETOOTH_SOURCE_DIR}/stack"
)

SET(EXECUTABLE_OUTPUT_PATH ${BLUETOOTH_SOURCE_DIR}/bin/)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/ipc/common SRC_IPC_COMMON)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/src SRC)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/tests/common_mock COMMON_MOCK)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/src/util SRC_UTIL)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/src/base SRC_BASE)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/src/hfp_hf SRC_HFP_HF)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/tests/hfp_hf/hf_mock SRC_HF_MOCK)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/tests/hfp_hf SRC_TEST_HFP_HF)
list(APPEND SRC_STACK ${BLUETOOTH_SOURCE_DIR}/stack/platform/src/packet.c
                      ${BLUETOOTH_SOURCE_DIR}/stack/platform/src/buffer.c
                      ${BLUETOOTH_SOURCE_DIR}/stack/platform/src/allocator.c)
list(APPEND SRC_STUB ${BLUETOOTH_SOURCE_DIR}/external/dependence_stub/passthrough/telephone_client_service.cpp
                     ${BLUETOOTH_SOURCE_DIR}/external/dependence_stub/passthrough/volume_service.cpp)

LINK_DIRECTORIES(${PROJECT_BINARY_DIR}/lib
    "${BLUETOOTH_SOURCE_DIR}/lib/"
    "${BLUETOOTH_SOURCE_DIR}/external/gtest/lib"
)

# Targets greeter_[async_](client|server)
ADD_EXECUTABLE(${TARGET_NAME}
    ${COMMON_MOCK}
    ${SRC}
    ${SRC_IPC_COMMON}
    ${SRC_HF_MOCK}
    ${SRC_HFP_HF}
    ${SRC_TEST_HFP_HF}
    ${SRC_UTIL}
    ${SRC_BASE}
    ${SRC_STACK}
    ${SRC_STUB}
)

ADD_DEFINITIONS("--coverage")

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
    --coverage
    -fprofile-arcs
    xml2
    btlog
    gmock
    gtest
    utilsecurec_shared
)
ENDIF(APPLE)

#display status messhfe for important variables
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