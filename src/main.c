#include "main.h"

#include <arpa/inet.h>
#include <ctype.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/_types/_ssize_t.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

/*

   socket(AF_INET, socket_type, protocol);
   socket types:
   SOCK_STREAM
   SOCK_DGRAM
   SOCK_RAW

   Valid socket types include SOCK_STREAM to open a stream socket,
   SOCK_DGRAM to open a datagram socket, and SOCK_RAW to open a
   raw(7) socket to access the IP protocol directly.

   connectx(int, const sa_endpoints_t *, sae_associd_t, unsigned int, const
   struct iovec *, unsigned int, size_t *, sae_connid_t *)

   connect(int, const struct

   sockaddr *, socklen_t) listen(int, int)

   AF_INET - ipv4
   AF_INET6 - ipv6

*/

void error(const char *msg) {
    fprintf(stderr, "Error: %s", msg);
    exit(EXIT_FAILURE);
}

typedef struct {
    uint8_t ip[4];
    uint16_t port;
} ServerAddressIPv4;

typedef struct {
    ServerAddressIPv4 addr;
    int server_fd;
} PlintServer;

void PlintServer_start(PlintServer *ps, const ServerAddressIPv4 saddr);

int main(void) {

    PlintServer ps = {0};
    ServerAddressIPv4 server_addr = {.ip = {127, 0, 0, 1}, .port = 6969};
    PlintServer_start(&ps, server_addr);

    printf("listening on http://%d.%d.%d.%d:%d\n", server_addr.ip[0],
            server_addr.ip[1], server_addr.ip[2], server_addr.ip[3],
            server_addr.port);

    for (;;) {
        struct sockaddr_in peer_addr = {0};
        socklen_t peer_addr_size = sizeof(peer_addr);
        int client_fd =
            accept(ps.server_fd, (struct sockaddr *)&peer_addr, &peer_addr_size);
        char message[] = "hello";

        char buf[1024];
        ssize_t len = recv(client_fd, buf, 1024, 0);
        if (len > 0) {
            for (ssize_t i = 0; i < len; i++) {
                printf("%c", buf[i]);
            }
        }

        send(client_fd, message, strlen(message), 0);

        shutdown(client_fd, SHUT_RD);
        close(client_fd);
    }

    return EXIT_SUCCESS;
}

// helper: PlintServer_start
intern_fn struct sockaddr_in
_PlintServer_init_socket_address(const ServerAddressIPv4 saddr) {
    in_addr_t ip[sizeof(struct in_addr)];
    int s;
    {
        char tmp[64] = {0};
        snprintf(tmp, 64, "%d.%d.%d.%d", saddr.ip[0], saddr.ip[1], saddr.ip[2],
                saddr.ip[3]);
        s = inet_pton(AF_INET, tmp, ip);
        if (s <= 0) {
            perror("inet_pton");
        }
    }
    return (struct sockaddr_in){
        .sin_family = AF_INET, .sin_port = htons(saddr.port), {ip[0]}};
}

void PlintServer_start(PlintServer *ps, const ServerAddressIPv4 saddr) {
    struct sockaddr_in addr = _PlintServer_init_socket_address(saddr);

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
        perror("socket");

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

#define LISTEN_BACKLOG 60
    if (listen(server_fd, LISTEN_BACKLOG) == -1)
        error("listen()");

    ps->server_fd = server_fd;
}
