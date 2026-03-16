ADD_LIBRARY(sys)

SET(
  SYS_SOURCES
    poller.cc
    
)


TARGET_LINK_LIBRARIES(
  sys
    INTERFACE
      core
      # pthread
)
