#include "buffer.h"

i32 buffer_t::commit_first_read(i32 size, char* data) {
    size_on_disk msg_size;
    msg_size.d_[0] = data[0];
    msg_size.d_[1] = data[1];
    msg_size.d_[2] = data[2];
    msg_size.d_[3] = data[3];
    buf_.resize(msg_size.size_ + 1);
    buf_.back() = 0;
    std::copy(data + 0,
              data + std::min(size, i32(msg_size.size_ + sizeof(i32))) + 1,
              buf_.begin());
    if (size >= msg_size.size_) {
        done_ = true;
        return msg_size.size_ + i32(sizeof(i32));
    }
    offset_ = size;
    done_ = false;
    return size;
}

i32 buffer_t::commit_next_read(i32 size, char* data) {
    i32 left_to_read = buf_.size() - offset_;
    std::copy(data + 0, data + std::min(size, left_to_read), buf_.begin()+offset_);
    if (size >= left_to_read) {
        done_ = true;
        return left_to_read;
    }

    offset_ += size;
    return size;
}

void buffer_t::take(buffer_t& recipient) {
    recipient = std::move(*this);
    *this = buffer_t();
}

bool buffer_t::is_done() const {
    return done_;
}

bool buffer_t::is_inited() const {
    return offset_ != 0;
}

const char* buffer_t::to_str() const {
    return &(buf_[sizeof(i32)]);
}
