#include <fstream>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <spdlog/spdlog.h>
#include "../core/entities.hpp"
#include "../sys/poller.hpp"
#include "../sys/fifo.hpp"
#include "../sys/fork_spawn.hpp"
#include "../sys/net/conn.hpp"
#include "../sys/net/listener.hpp"

/* i just invented my own io_context... omg.. */
struct ServerContext {};
