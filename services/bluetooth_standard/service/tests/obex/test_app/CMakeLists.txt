CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(obex_test_app CXX C)

INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/."
	"${BLUETOOTH_SOURCE_DIR}/external/gtest/include"
	"${BLUETOOTH_SOURCE_DIR}/external/digest/src/md5"
  "${BLUETOOTH_SOURCE_DIR}/external/digest/include"  
	"${BLUETOOTH_SOURCE_DIR}/external/libxml2/include"
	"${BLUETOOTH_SOURCE_DIR}/external/log/include"
	"${BLUETOOTH_SOURCE_DIR}/external/dependence_stub/include"
  "${BLUETOOTH_SOURCE_DIR}/external/ohos_util/include"    
	"${BLUETOOTH_SOURCE_DIR}/framework/include"
	"${BLUETOOTH_SOURCE_DIR}/service/src/common"
	"${BLUETOOTH_SOURCE_DIR}/service/src/base"
	"${BLUETOOTH_SOURCE_DIR}/service/src/util"
	"${BLUETOOTH_SOURCE_DIR}/service/src/classic"
  "${BLUETOOTH_SOURCE_DIR}/stack/include"
	"${BLUETOOTH_SOURCE_DIR}/stack/platform/include"
	"${BLUETOOTH_SOURCE_DIR}"
	"${BLUETOOTH_SOURCE_DIR}/stack"
	"${BLUETOOTH_SOURCE_DIR}/service/include"
	"${BLUETOOTH_SOURCE_DIR}/service/src"
	"${BLUETOOTH_SOURCE_DIR}/service/src/obex"
  "${BLUETOOTH_SOURCE_DIR}/service/src/transport"
  "${BLUETOOTH_SOURCE_DIR}/common"    
  "${BLUETOOTH_SOURCE_DIR}/ipc/common"    

)

SET(EXECUTABLE_OUTPUT_PATH ${BLUETOOTH_SOURCE_DIR}/bin/)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/external/digest/src/md5 SRC_MD5)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/stack/platform/src SRC_STACK)

AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/src/util SRC_UTIL)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/src/common SRC_COMMON)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/src/classic SRC_CLASSIC)


AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/src/obex SRC_OBEX)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/src/base SRC_BASE)
#AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/src/sock SRC_SOCK)

AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/tests/obex/sock_mock SOCK_MOCK)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/service/tests/obex/obex_gap SRC_OBEX_GAP)
AUX_SOURCE_DIRECTORY(${BLUETOOTH_SOURCE_DIR}/ipc/common SRC_IPC)

list(APPEND SRC_STACK_TEST ${BLUETOOTH_SOURCE_DIR}/stack/platform/src/packet.c ${BLUETOOTH_SOURCE_DIR}/stack/platform/src/buffer.c ${BLUETOOTH_SOURCE_DIR}/stack/platform/src/allocator.c)
list(APPEND SRC_DIGEST ${BLUETOOTH_SOURCE_DIR}/external/digest/src/message_digest.cpp)
list(APPEND SRC_SOCK ${BLUETOOTH_SOURCE_DIR}/service/src/transport/transport_factory.cpp
${BLUETOOTH_SOURCE_DIR}/service/src/transport/transport_l2cap.cpp
${BLUETOOTH_SOURCE_DIR}/service/src/transport/transport_rfcomm.cpp )
list(REMOVE_ITEM SRC_UTIL EXCLUDE ${BLUETOOTH_SOURCE_DIR}/service/src/util/dispatcher.cpp)
list(APPEND SRC_UTIL ${BLUETOOTH_SOURCE_DIR}/service/tests/obex/util_mock/dispatcher.cpp)

LINK_DIRECTORIES(${PROJECT_BINARY_DIR}/lib
    "${BLUETOOTH_SOURCE_DIR}/lib/"
    "${BLUETOOTH_SOURCE_DIR}/external/gtest/lib"
)
IF(${ENABLE_ICONV} STREQUAL "ON")
    LINK_DIRECTORIES("${BLUETOOTH_SOURCE_DIR}/external/libiconv/lib")
ENDIF()

# Targets greeter_[async_](client|server)
ADD_EXECUTABLE(obex_client "./main_obex_client_test.cpp"
  ${SRC_SOCK}
  ${SRC_COMMON}
  ${SRC_OBEX_GAP}
  ${SRC_CLASSIC}
  ${SRC_OBEX}
  ${SRC_STACK}
  ${SRC_UTIL}
  ${SRC_BASE}
  ${SRC_DIGEST}
  ${SRC_MD5}
  ${SRC_LOG}
  ${SRC_PUGIXML}
  ${SRC_IPC}
)
ADD_EXECUTABLE(obex_server "./main_obex_server_test.cpp"
  ${SRC_SOCK}
  ${SRC_COMMON}
  ${SRC_OBEX_GAP}
  ${SRC_CLASSIC}
  ${SRC_OBEX}
  ${SRC_STACK}
  ${SRC_UTIL}
  ${SRC_BASE}
  ${SRC_DIGEST}
  ${SRC_MD5}
  ${SRC_LOG}
  ${SRC_PUGIXML}
  ${SRC_IPC}

)
ADD_EXECUTABLE(obex_client_mock "./main_obex_client_test.cpp"
  ${SOCK_MOCK}
  ${SRC_OBEX}
  ${SRC_STACK_TEST}
  ${SRC_UTIL}
  ${SRC_BASE}
  ${SRC_DIGEST}
  ${SRC_MD5}
  ${SRC_LOG}
  ${SRC_PUGIXML}
  ${SRC_IPC}

)
ADD_EXECUTABLE(obex_server_mock "./main_obex_server_test.cpp"
  ${SOCK_MOCK}
  ${SRC_OBEX}
  ${SRC_STACK_TEST}
  ${SRC_UTIL}
  ${SRC_BASE}
  ${SRC_DIGEST}
  ${SRC_MD5}
  ${SRC_LOG}
  ${SRC_PUGIXML}
  ${SRC_IPC}

)
IF(APPLE)
target_link_libraries(tmp
  clang_rt.profile_osx
  -lpthread
  gmock
  gtest)
ELSE(LINUX)

target_link_libraries(obex_client
  dl
  stdc++
  pthread
  btstack
  btservice
  btlog
  dependencestub
  utilsecurec_shared
)
target_link_libraries(obex_server
  dl
  stdc++
  pthread
  btstack
  btservice
  btlog
  dependencestub
  utilsecurec_shared
)

target_compile_definitions(obex_client_mock PRIVATE IS_SOCK_MOCK)
target_link_libraries(obex_client_mock
  dl
  stdc++
  pthread
  btlog
  xml2
  util
  utilsecurec_shared
)

target_compile_definitions(obex_server_mock PRIVATE IS_SOCK_MOCK)
target_link_libraries(obex_server_mock
  dl
  stdc++
  pthread
  btlog
  xml2
  util
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