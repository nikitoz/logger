#include "netw.h"
#include "buffer.h"
#include "common.h"

struct netw_config_t {
    int port_;
};

static netw_config_t global_config = {8978};

buffer_t& netw_t::get_buffer_for_fd(int fd) { return client_buffers_[fd]; }

i32 netw_t::read_from_client(int fd) {
    char static_buffer[MAXMSG] = {0};
    buffer_t& buffer = this->get_buffer_for_fd(fd);
    const i32 size_read = recv(fd, static_buffer, MAXMSG, 0);
    if (size_read < 0) {
        perror("read error");
        return -1;
    }
    static_buffer[size_read] = 0;
    i32 offset = 0;
    while (offset < size_read) {
        i32 taken = buffer.is_inited()
                        ? buffer.commit_next_read(size_read, static_buffer)
                        : buffer.commit_first_read(size_read, static_buffer);
        fprintf(stdout, "Server: message: `%s'\n", buffer.to_str());
        if (buffer.is_done()) {
            // commit_buffer, create new one
            buffer_t recipient;
            buffer.take(recipient);
            fprintf(stdout, "Server: committed message: `%s'\n",
                    recipient.to_str());
        }
        offset += taken;
    }
    return 0;
}

int netw_t::make_socket(uint16_t port) {
    int sock;
    struct sockaddr_in name;
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return -1;
    }
    name.sin_family = AF_INET;
    name.sin_port = htons(port);
    name.sin_addr.s_addr = htonl(INADDR_ANY);
    int reuse = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse));
    if (bind(sock, (struct sockaddr*)&name, sizeof(name)) < 0) {
        perror("bind");
        return -1;
    }

    return sock;
}
i32 netw_t::tcp_main() {
    int sock;
    fd_set active_fd_set, select_fd_set;
    sockaddr_in clientname;
    socklen_t size;

    /* Create the socket and set it up to accept connections. */
    sock = make_socket(global_config.port_);
    if (listen(sock, 1) < 0) {
        perror("listen");
        return -1;
    }
    int thread_num = 0;
    (void)thread_num;
    /* Initialize the set of active sockets. */
    FD_ZERO(&active_fd_set);
    FD_SET(sock, &active_fd_set);

    for (;;) {
        select_fd_set = active_fd_set;
        if (select(FD_SETSIZE, &select_fd_set, NULL, NULL, NULL) < 0) {
            perror("select");
            return -1;
        }

        for (int i = 0; i != FD_SETSIZE; ++i) {
            if (!FD_ISSET(i, &select_fd_set)) {
                continue;
            }
            if (i == sock) {
                /* Connection request on original socket. */
                int newfd = -1;
                size = sizeof(clientname);
                newfd = accept(sock, (struct sockaddr*)&clientname, &size);
                if (newfd < 0) {
                    perror("accept");
                    return -1;
                }
                fprintf(stderr, "Server: connect from host %s, port %hd.\n",
                        inet_ntoa(clientname.sin_addr),
                        ntohs(clientname.sin_port));
                FD_SET(newfd, &active_fd_set);
            } else if (this->read_from_client(i) != 0) {
                fprintf(stdout, "Failed read\n");
                FD_CLR(i, &active_fd_set);
                close(i);
            }
        }
    }
}
