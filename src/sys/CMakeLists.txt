SET(
  SYS_SOURCES
    poller.cc
    signal.cc
    fifo.cc
    net/listener.cc
    net/conn.cc
    fork_spawn.cc
)

ADD_LIBRARY(
  sys
    STATIC
      ${SYS_SOURCES}
)

TARGET_LINK_LIBRARIES(
  sys
    PUBLIC
      core
    PRIVATE
      spdlog::spdlog
)
