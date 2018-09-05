#include "netw_replicator.h"

netw_replicator_t::netw_replicator_t(const char *hostname, int portno) {
    struct sockaddr_in serveraddr;
    struct hostent *server;

    fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd_ < 0) {
        perror("Error");
        fd_ = -1;
        return;
    }

    server = ::gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr, "Error: no such host as %s\n", hostname);
        fd_ = -1;
        return;
    }

    ::bzero((char *)&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    ::bcopy((char *)server->h_addr, (char *)&serveraddr.sin_addr.s_addr,
            server->h_length);
    serveraddr.sin_port = ::htons(portno);

    if (::connect(fd_, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) <
        0) {
        perror("Error");
        fd_ = -1;
        return;
    }
}

i32 netw_replicator_t::replicate(i32 in_fd, off_t offset, i32 size) {
    if (fd_ == -1 || in_fd < 0) return -1;
    i32 res = ::sendfile(fd_, in_fd, &offset, size);
    if (res < 0) {
        perror("Error");
        return -1;
    }
    return 0;
}
