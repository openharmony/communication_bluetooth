CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(a2dp_snk_test CXX C)

SET(TARGET_NAME a2dp_snk_test)
SET (CMAKE_C_COMPILER gcc)
SET (CMAKE_CXX_COMPILER g++)

INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/."
  "${BLUETOOTH_SOURCE_DIR}/service/tests/common_mock"
  "${BLUETOOTH_SOURCE_DIR}/external/gtest/googletest/include"	
	"${BLUETOOTH_SOURCE_DIR}/external/gtest/googlemock/include"
  "${BLUETOOTH_SOURCE_DIR}/external/libxml2/include"
  "${BLUETOOTH_SOURCE_DIR}/external/log/include"
  "${BLUETOOTH_SOURCE_DIR}/framework/include"
  "${BLUETOOTH_SOURCE_DIR}/service/include"
  "${BLUETOOTH_SOURCE_DIR}/common"
	"${BLUETOOTH_SOURCE_DIR}/service/src/base"
	"${BLUETOOTH_SOURCE_DIR}/service/src/util"
	"${BLUETOOTH_SOURCE_DIR}/stack/include"
  "${BLUETOOTH_SOURCE_DIR}/stack"
  "${BLUETOOTH_SOURCE_DIR}/service/src/"
  "${BLUETOOTH_SOURCE_DIR}/service/src/a2dp_snk"
	"${BLUETOOTH_SOURCE_DIR}/service/tests/a2dp_snk"
  "${BLUETOOTH_SOURCE_DIR}/ipc/common"
  "${BLUETOOTH_SOURCE_DIR}/external/dependence_stub/include"
  "${BLUETOOTH_SOURCE_DIR}/external/dependence_stub/include/stub"
  "${BLUETOOTH_SOURCE_DIR}/service/src/gavdp/a2dp_codec/include"
  "${BLUETOOTH_SOURCE_DIR}/service/src/gavdp/a2dp_codec/aaccodecctrl/include"
  "${BLUETOOTH_SOURCE_DIR}/service/src/gavdp/a2dp_codec/sbccodecctrl/include"
  "${BLUETOOTH_SOURCE_DIR}/service/src/gavdp/a2dp_codec/sbclib/include"
  "${BLUETOOTH_SOURCE_DIR}/service/src/base/"
  "${BLUETOOTH_SOURCE_DIR}/service/src/gavdp/"
  "${BLUETOOTH_SOURCE_DIR}/external/aac/libAACdec/include"
  "${BLUETOOTH_SOURCE_DIR}/external/aac/libAACenc/include"
  "${BLUETOOTH_SOURCE_DIR}/external/aac/libArithCoding/include"
  "${BLUETOOTH_SOURCE_DIR}/external/aac/libDRCdec/include"
  "${BLUETOOTH_SOURCE_DIR}/external/aac/libFDK/include"
  "${BLUETOOTH_SOURCE_DIR}/external/aac/libMpegTPDec/include"
  "${BLUETOOTH_SOURCE_DIR}/external/aac/libMpegTPEnc/include"
  "${BLUETOOTH_SOURCE_DIR}/external/aac/libPCMutils/include"
  "${BLUETOOTH_SOURCE_DIR}/external/aac/libSACdec/include"
  "${BLUETOOTH_SOURCE_DIR}/external/aac/libSACenc/include"
  "${BLUETOOTH_SOURCE_DIR}/external/aac/libSBRdec/include"
  "${BLUETOOTH_SOURCE_DIR}/external/aac/libSBRenc/include"
  "${BLUETOOTH_SOURCE_DIR}/external/aac/libSYS/include"
  "${BLUETOOTH_SOURCE_DIR}/external/ohos_util/include"
)

SET(EXECUTABLE_OUTPUT_PATH ${BLUETOOTH_SOURCE_DIR}/bin/)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/src/util SRC_UTIL)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/src/base SRC_BASE)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/tests/common_mock COMMON_MOCK)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/src/a2dp_src SRC_A2DP_SRC)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/src/a2dp_snk SRC_A2DP_SNK)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/src/gavdp SRC_A2DP_GAVDP)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/tests/a2dp_snk/snk_mock SRC_SNK_MOCK)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/tests/a2dp_snk SRC_TEST_A2DP_SNK)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/ipc/common SRC_IPC_COMMON)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/src/gavdp/a2dp_codec SRC_A2DP_CODEC)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/src/gavdp/a2dp_codec/aaccodecctrl/src SRC_A2DP_AAC)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/src/gavdp/a2dp_codec/sbccodecctrl/src SRC_A2DP_SBC)
#AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/src/gavdp/a2dp_codec/sbclib SRC_A2DP_CODEC_SBC)
list(APPEND SRC_STACK ${BLUETOOTH_SOURCE_DIR}/stack/platform/src/packet.c
                      ${BLUETOOTH_SOURCE_DIR}/stack/platform/src/buffer.c
                      ${BLUETOOTH_SOURCE_DIR}/stack/platform/src/allocator.c)
list(APPEND SRC_STUB ${BLUETOOTH_SOURCE_DIR}/external/dependence_stub/passthrough/a2dp_data_service.cpp)

LINK_DIRECTORIES(${PROJECT_BINARY_DIR}/lib
    "${BLUETOOTH_SOURCE_DIR}/lib/"
    "${BLUETOOTH_SOURCE_DIR}/external/gtest/lib"
)

ADD_DEFINITIONS("--coverage")

# Targets greeter_[async_](client|server)
ADD_EXECUTABLE(${TARGET_NAME}
  ${COMMON_MOCK}
  ${SRC_SNK_MOCK}
  ${SRC_TEST_A2DP_SNK}
  ${SRC_A2DP_SNK}
  ${SRC_A2DP_SRC}
  ${SRC_A2DP_GAVDP}
  ${SRC_UTIL}
  ${SRC_BASE}
  ${SRC_IPC_COMMON}
  ${SRC_STACK}
  ${SRC_STUB}
  ${SRC_A2DP_CODEC}
  ${SRC_A2DP_AAC}
  ${SRC_A2DP_SBC}
  ${SRC_A2DP_CODEC_SBC}
)

IF(APPLE)
TARGET_LINK_LIBRARIES(${TARGET_NAME}
  clang_rt.profile_osx
  -lpthread
  gmock
  gtest)
ELSE(LINUX)
TARGET_LINK_LIBRARIES(${TARGET_NAME}
  stdc++
  pthread
  gcov
  -fprofile-arcs
  --coverage
  xml2
  btlog
  gmock
  gtest
  utilsecurec_shared
  dl
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
