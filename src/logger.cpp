#include "logger.h"
#include "netw.h"

context_t::context_t(const char* name, const char* base_path) {
    this->name_ = name;
    this->base_path_ = base_path;
}

context_t::~context_t() {
}

i32 context_t::make_logger(const char* name, i32 id, logger_t* logger) {
    if (!logger) {
        fprintf(stderr, "logger is NULL\n");
        return 1;
    }

    std::string file_path;
    file_path.reserve(base_path_.size()+2+strlen(name));
    file_path += this->base_path_;
    file_path += "/";
    file_path += name;
    fprintf(stderr, "Creating file %s\n", file_path.c_str());

    int fd = open(file_path.c_str(), O_RDONLY | O_CREAT, 0666);
    if (fd < 0) {
        perror("Error ");
        fprintf(stderr, "Could not open logger %s\n", file_path.c_str());
        return 1;
    }
    off_t filesize = lseek(fd, 0L, SEEK_END);
    lseek(fd, 0L, SEEK_SET);
    logger->fd_ = fd;
    logger->file_path_ = std::move(file_path);
    logger->filesize_ = filesize;
    return 0;
}

i32 logger_t::make_reader(reader_t* reader) {
    if (!reader) {
        fprintf(stderr, "Reader is NULL!\n");
        return 1;
    }
    reader->fd_ = dup(this->fd_);
    reader->file_path_ = this->file_path_;
    return 0;
}

i32 logger_t::make_appender(appender_t* appender) {
    const char* path = this->file_path_.c_str();
    int afd = open(path, O_APPEND | O_RDWR);
    if (afd < 0) {
        fprintf(stderr, "Cannot open file %s for append! 1\n", path);
        return 1;
    }
    fprintf(stderr, "Make appender fd=%d\n", afd);
    appender->file_path_ = path;
    appender->fd_ = afd;
    appender->filesize_ = this->filesize_;
    return 0;
}

logger_t::~logger_t() {
    i32 close_err = close(this->fd_);
    if (close_err == -1) {
        fprintf(stderr, "Kill logger %s\n", this->file_path_.c_str());
    }
}

i32 appender_t::append(const char* data, i32 size) {
    if (!data || this->fd_ < 0 || size <= 0) {
        fprintf(stderr, "Write is not possible: file %s data %s!",
                this->file_path_.c_str(), data ? data : "");
        return 1;
    }
    size_on_disk first_4;
    first_4.size_ = size;
    std::vector<char> buffer(size+sizeof(i32));
    char* wdata = buffer.data();
    wdata[0] = first_4.d_[0];
    wdata[1] = first_4.d_[1];
    wdata[2] = first_4.d_[2];
    wdata[3] = first_4.d_[3];
    strncpy(wdata + 4, data, size);
    const i32 full_size = size+4;
    if (!exp_backoff(10,
                     [&]() { return write(this->fd_, wdata, full_size) > 0; })) {
        fprintf(stderr, "Write fd %d , errno %d\n", this->fd_, errno);
        return -1;
    }

    replicate_(this->fd_, this->filesize_, full_size);
    this->filesize_ += full_size;

    return 0;
}

i32 appender_t::append(const char* data) {
    return this->append(data, (i32)strlen(data));
}

i32 appender_t::set_replicator(fn_replicate_t&& replicate) {
    replicate_ = std::move(replicate);
    return 0;
}

appender_t::~appender_t() {
    i32 fsync_err = fsync(this->fd_);
    if (fsync_err == -1) {
        fprintf(stderr,
                "Kill appender fsync result %d, trying to close file anyway\n",
                fsync_err);
    }
    i32 close_err = close(this->fd_);
    if (close_err == -1) {
        fprintf(stderr, "Kill appender close result %d\n", errno);
    }
}

i32 reader_t::consume_one_str(char* data, ssize_t max_size) {
    i32 size = 0;
    ssize_t read_res = read(this->fd_, &size, sizeof(i32));
    if (read_res == 0) {
        fprintf(stderr, "File %s is empty\n", this->file_path_.c_str());
        return 1;
    } else if (read_res != sizeof(i32)) {
        fprintf(stderr, "Could not read i32 size from %s\n", this->file_path_.c_str());
        return 1;
    }
    ssize_t read_bytes = 0;
    i32 delay = 0;
    read_res = read(this->fd_, data, size - read_bytes);
    read_bytes += read_res < 0 ? 0 : read_res;
    while (read_bytes != size && delay < 30) {
        sleep(1 << (delay++));
        read_res = read(this->fd_, data + read_bytes, size - read_bytes);
        read_bytes += read_res < 0 ? 0 : read_res;
    }
    if (read_bytes != size) {
        fprintf(stderr, "Could not read actual data of size %d from file %s\n",
                size, this->file_path_.c_str());
        return 2;
    }
    return 0;
}

reader_t::~reader_t() {
    i32 close_res = close(this->fd_);
    if (close_res == -1) {
        fprintf(stderr, "Could not close reader descriptor %d", errno);
    }
}
