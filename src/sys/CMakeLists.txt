SET(
  SYS_SOURCES
    poller.cc
    signal.cc
    fifo_guard.cc
    net/listener.cc
    net/conn.cc
    fork_spawn.cc
    ipc.cc
    pipe_sink.cc
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
      utils
    PRIVATE
      spdlog
)
