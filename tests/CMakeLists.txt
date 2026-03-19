SET(CMAKE_BUILD_TYPE Debug)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

# ADD_SUBDIRECTORY(...)

CONFIGURE_FILE(
  ${CMAKE_SOURCE_DIR}/config.json
  ${CMAKE_CURRENT_BINARY_DIR}/config.json
  COPYONLY
)


SET(
  TEST_SOURCES
    unit.cc
)

ADD_EXECUTABLE(unitests ${TEST_SOURCES})

# *---*---*---*---*---*---*---*---*---*---*---*

FIND_PACKAGE(GTest REQUIRED)
INCLUDE(GoogleTest)

TARGET_LINK_LIBRARIES(
  unitests
    PRIVATE
      GTest::gtest_main
      core
      utils
)

GTEST_DISCOVER_TESTS(unitests)
