#pragma once
#include "common.h"
#include "buffer.h"

struct netw_t {
    i32 tcp_main();
    i32 read_from_client(int fd);
    int make_socket(uint16_t port);
    buffer_t& get_buffer_for_fd(int fd);

   private:
    std::unordered_map<int, buffer_t> client_buffers_;
};
