#pragma once
#include "common.h"

struct logger_t;
struct appender_t;
struct reader_t;

struct context_t {
    context_t(const char* name, const char* base_path);
    i32 make_logger(const char* name, i32 id, logger_t* logger);
    std::string name_;
    std::string base_path_;
    ~context_t();
};

struct logger_t {
    i32 make_reader(reader_t* reader);
    i32 make_appender(appender_t* appender);
    ~logger_t();
    int fd_ = -1;
    std::string file_path_;
    off_t filesize_ = 0;
};

struct appender_t : public logger_t {
    i32 append(const char* data);
    i32 append(const char* data, i32 size);
    i32 set_replicator(fn_replicate_t&& replicate);
    ~appender_t();
private:
    fn_replicate_t replicate_ = dummy_replication;
};

struct reader_t : public logger_t {
    i32 consume_one_str(char* data, ssize_t max_size);
    ~reader_t();
};
