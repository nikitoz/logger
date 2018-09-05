#pragma once
#include "common.h"

struct netw_replicator_t {
    netw_replicator_t(const char* hostname, int portno);
    i32 replicate(i32 in_fd, off_t offset, i32 size);

private:
    i32 fd_ = -1;
};
