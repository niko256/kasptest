ADD_LIBRARY(sys)

SET(
  SYS_SOURCES
    poller.cc
    signal.cc
    fifo.cc

)


TARGET_LINK_LIBRARIES(
  sys
    INTERFACE
      core
      # pthread
)
