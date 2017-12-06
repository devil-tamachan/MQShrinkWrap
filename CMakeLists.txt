cmake_minimum_required(VERSION 3.8)
project (MQShrinkWrap)

add_definitions(-D_UNICODE -DUNICODE)

add_definitions( -DBOOST_ALL_NO_LIB )
set( Boost_USE_STATIC_LIBS   ON )
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

IF(WIN32)
  add_library(MQShrinkWrap SHARED
    MQShrinkWrap.cpp
    ${MQSDK_INCLUDE_DIR}/MQInit.cpp
    ${MQSDK_INCLUDE_DIR}/MQPlugin.cpp
    ${MQSDK_INCLUDE_DIR}/MQWidget.cpp
  )
  set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /OPT:REF")
ELSE()
  set(MACOSX_BUNDLE_INFO_STRING "tamachan.mq.MQShrinkWrap")
  set(MACOSX_BUNDLE_BUNDLE_NAME "tamachan.mq.MQShrinkWrap")
  set(MACOSX_BUNDLE_GUI_IDENTIFIER "tamachan.mq.MQShrinkWrap")

  add_library(MQShrinkWrap MODULE
    MQShrinkWrap.cpp
    ${MQSDK_INCLUDE_DIR}/MQInit.cpp
    ${MQSDK_INCLUDE_DIR}/MQPlugin.cpp
    ${MQSDK_INCLUDE_DIR}/MQWidget.cpp
    ${MQSDK_INCLUDE_DIR}/osx/StringUtil.mm
  )
  target_link_libraries(MQShrinkWrap "-framework CoreFoundation")
  target_link_libraries(MQShrinkWrap "-framework Cocoa")
  set_target_properties(MQShrinkWrap PROPERTIES
    BUNDLE TRUE
    BUNDLE_EXTENSION plugin
  )
ENDIF()

if(CMAKE_CL_64)
  set_target_properties(MQShrinkWrap PROPERTIES OUTPUT_NAME "MQShrinkWrap64")
endif()

target_link_libraries( MQShrinkWrap ${Boost_LIBRARIES} )