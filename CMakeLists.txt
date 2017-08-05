cmake_minimum_required(VERSION 3.8)
project (MQShrinkWrap)

add_definitions(-D_UNICODE -DUNICODE)

add_definitions( -DBOOST_ALL_NO_LIB )
find_package(Boost 1.64.0 COMPONENTS program_options filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIR})

set(MQSDK_DIR "" CACHE PATH "MQInit.cpp")

if(EXISTS ${MQSDK_DIR}/mqsdk/MQInit.cpp)
  set(MQSDK_INCLUDE_DIR ${MQSDK_DIR}/mqsdk/)
elseif(EXISTS ${MQSDK_DIR}/MQInit.cpp)
  set(MQSDK_INCLUDE_DIR ${MQSDK_DIR})
endif()

if(NOT DEFINED MQSDK_INCLUDE_DIR)
  message(FATAL_ERROR "MQSDK_DIR not found!")
endif()

INCLUDE_DIRECTORIES( "${MQSDK_INCLUDE_DIR}" )

add_library(MQShrinkWrap SHARED
  MQShrinkWrap.cpp
  ${MQSDK_INCLUDE_DIR}/MQInit.cpp
  ${MQSDK_INCLUDE_DIR}/MQPlugin.cpp
  ${MQSDK_INCLUDE_DIR}/MQWidget.cpp
)
target_link_libraries( MQShrinkWrap ${Boost_LIBRARIES} )