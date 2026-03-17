ADD_EXECUTABLE(server server.cc)
TARGET_LINK_LIBRARIES(server PRIVATE sys utils spdlog::spdlog)

ADD_EXECUTABLE(client client.cc)
TARGET_LINK_LIBRARIES(client PRIVATE sys utils spdlog::spdlog)

ADD_EXECUTABLE(stats stats.cc)
TARGET_LINK_LIBRARIES(stats PRIVATE sys utils spdlog::spdlog)
