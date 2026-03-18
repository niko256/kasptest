INCLUDE(FetchContent)

# *---*---*---*---*---*---*---*---*---*---*---*

# tracing
FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG v1.17.0
  GIT_SHALLOW TRUE
)

# *---*---*---*---*---*---*---*---*---*---*---*

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
    GIT_SHALLOW TRUE
)

# *---*---*---*---*---*---*---*---*---*---*---*

FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.12.0
  GIT_SHALLOW TRUE
)

# *---*---*---*---*---*---*---*---*---*---*---*

FetchContent_MakeAvailable(googletest json spdlog)
