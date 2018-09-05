#pragma once
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <aio.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/sendfile.h>

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <queue>
#include <string>
#include <unordered_map>
#include <bits/stdc++.h>

#define i32 int32_t
#define u8 uint8_t
const int MAXMSG = 256;
union size_on_disk {
    char d_[4];
    i32 size_;
};

using fn_replicate_t = std::function<i32 (i32, off_t, i32)>;

i32 dummy_replication(i32 in_fd, off_t offset, i32 size);

bool exp_backoff(i32 backoff_max, std::function<bool()> func);
