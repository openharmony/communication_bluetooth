CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(gatt_service_test CXX C)

add_definitions(--coverage)
SET(TARGET_NAME gatt_test)

INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/."
  "${BLUETOOTH_SOURCE_DIR}/ipc/binder"
  #"${BLUETOOTH_SOURCE_DIR}/ipc/common"
  "${BLUETOOTH_SOURCE_DIR}/external/gtest/googletest/include"	
	"${BLUETOOTH_SOURCE_DIR}/external/gtest/googlemock/include"
	"${BLUETOOTH_SOURCE_DIR}/external/log/include"
  "${BLUETOOTH_SOURCE_DIR}/framework/include"
  "${BLUETOOTH_SOURCE_DIR}/service/include"
#  "${BLUETOOTH_SOURCE_DIR}/service/src/common"
  "${BLUETOOTH_SOURCE_DIR}/service/tests/common_mock"
	"${BLUETOOTH_SOURCE_DIR}/service/src/base"
	"${BLUETOOTH_SOURCE_DIR}/service/src/util"
	"${BLUETOOTH_SOURCE_DIR}/service/src/classic"
	"${BLUETOOTH_SOURCE_DIR}/service/src/ble"
  "${BLUETOOTH_SOURCE_DIR}/service/tests/gatt/mock"
	"${BLUETOOTH_SOURCE_DIR}/service/src/gatt"
	"${BLUETOOTH_SOURCE_DIR}/service/src"
	"${BLUETOOTH_SOURCE_DIR}/stack/include"
  "${BLUETOOTH_SOURCE_DIR}/stack/platform/include"
  "${BLUETOOTH_SOURCE_DIR}/stack"
  "${BLUETOOTH_SOURCE_DIR}/common"
  "${BLUETOOTH_SOURCE_DIR}/stack/src"
  "${BLUETOOTH_SOURCE_DIR}/external/boringssl/include"
  "${BLUETOOTH_SOURCE_DIR}/external/ohos_util/include"
)

SET(EXECUTABLE_OUTPUT_PATH ${BLUETOOTH_SOURCE_DIR}/bin/)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/src/common SRC_COMMON)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/src/base SRC_BASE)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/src/util SRC_UTIL)
#AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/stack/src/btm SRC_BTM)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/stack/include SRC_INC)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/src/gatt SRC_GATT)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/tests/common TEST_COMMON)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/tests/gatt SRC_TEST_GATT)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/tests/gatt/mock TEST_MOCK)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/tests/common_mock COMMON_MOCK)

LINK_DIRECTORIES(${PROJECT_BINARY_DIR}/lib
    "${BLUETOOTH_SOURCE_DIR}/lib/"
    "${BLUETOOTH_SOURCE_DIR}/external/gtest/lib/"
)

ADD_DEFINITIONS("--coverage")

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-parameter")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter -Wno-effc++ -Wno-missing-field-initializers")

# Targets greeter_[async_](client|server)
ADD_EXECUTABLE(${TARGET_NAME}
  ${COMMON_MOCK}
  #${SRC_UTIL}
  ${SRC_BASE}
  #${SRC_PUGIXML}
  #${SRC_COMMON}
  #${SRC_STA}
  #${SRC_GATT}
  #${SRC_INC}
  ${TEST_MOCK}
  #${BLUETOOTH_SOURCE_DIR}/service/src/util/state_machine.cpp
  ${BLUETOOTH_SOURCE_DIR}/service/src/gatt/gatt_connection_manager.cpp
  ${BLUETOOTH_SOURCE_DIR}/service/src/gatt/gatt_client_service.cpp
  ${BLUETOOTH_SOURCE_DIR}/service/src/gatt/gatt_client_profile.cpp
  ${BLUETOOTH_SOURCE_DIR}/service/src/gatt/gatt_database.cpp
  ${BLUETOOTH_SOURCE_DIR}/service/src/gatt/gatt_cache.cpp
  ${BLUETOOTH_SOURCE_DIR}/service/src/gatt/gatt_service_base.cpp
  ${BLUETOOTH_SOURCE_DIR}/service/src/gatt/gatt_server_service.cpp
  ${BLUETOOTH_SOURCE_DIR}/service/src/gatt/gatt_server_profile.cpp
  ${BLUETOOTH_SOURCE_DIR}/service/src/gatt/gatt_based_services_manager.cpp
  ${BLUETOOTH_SOURCE_DIR}/service/src/gatt/dis/device_information_service.cpp
  ${BLUETOOTH_SOURCE_DIR}/service/src/gatt/gas/generic_access_service.cpp
  ${BLUETOOTH_SOURCE_DIR}/service/src/gatt/gatts/generic_attribute_service.cpp
  ${BLUETOOTH_SOURCE_DIR}/service/src/gatt/gatts/gatt_service_over_bredr.cpp
  ${BLUETOOTH_SOURCE_DIR}/service/src/gatt/dis/di_service_over_bredr.cpp
  ${SRC_TEST_GATT}
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
  btlog
  gmock
  gtest
  crypto
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