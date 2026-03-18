SET(
  UTILS_SOURCES
    repr.cc
    format.cc
)

ADD_LIBRARY(
  utils
    STATIC
      ${UTILS_SOURCES}
)

TARGET_INCLUDE_DIRECTORIES(
  utils
    PUBLIC
      ${CMAKE_CURRENT_SOURCE_DIR}/..
)

FIND_PACKAGE(nlohmann_json REQUIRED)

TARGET_LINK_LIBRARIES(
  utils
    PUBLIC
      core
    PRIVATE
      nlohmann_json::nlohmann_json
)
