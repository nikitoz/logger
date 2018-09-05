#pragma once
#include "common.h"

struct buffer_t {
    char* addr();
    i32 commit_first_read(i32 size, char* data);
    i32 commit_next_read(i32 size, char* data);
    bool is_done() const;
    bool is_inited() const;
    const char* to_str() const;
    void take(buffer_t& recipient);

private:
    std::vector<char> buf_;
    size_t offset_ = 0;
    size_t left_to_read_ = 0;
    bool done_ = false;
};
