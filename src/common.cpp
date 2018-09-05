#include "common.h"

bool exp_backoff(i32 backoff_max, std::function<bool()> func) {
    i32 backoff = 0;
    while (func() == false && backoff < backoff_max) {
        sleep(1 << (backoff++));
    }
    if (backoff == backoff_max) {
        perror("Error: exp_backoff");
        return false;
    }
    return true;
}

auto dummy_replication(i32 in_fd, off_t offset, i32 size) -> i32 { return size; }
