#include "main.h"

#include <arpa/inet.h>
#include <ctype.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// #define PLINT_NO_LOG

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

typedef struct {
    uint8_t ip[4];
    uint16_t port;
} ServerAddressIPv4;

#define _PLINT_MAX_ROUTES 100

typedef struct {
    char *route;
    char *file_path;
} PlintRoute;

typedef struct {
    ServerAddressIPv4 addr;
    int server_fd;
    PlintRoute route[_PLINT_MAX_ROUTES];
    uint n_routes;
} PlintServer;

void Plint_server_start(PlintServer *ps, const ServerAddressIPv4 saddr);

/*

   1. Create server on the stack
   PlintServer ps = {0};

   2. Append route structs to it
   (create route struct here that contains paths to html and whatever else)
   PlintServer_route_append(&ps, route)

   3. Define an address
   ServerAddressIPv4 server_addr = {.ip = {127, 0, 0, 1}, .port = 6969};

   4. Start the server
   Plint_server_start(&ps, server_addr);

*/

void PlintServer_append_route(PlintServer *ps, PlintRoute *pr) {
    if (ps->n_routes + 1 > _PLINT_MAX_ROUTES) {
        perror("max routes exceeded");
        exit(EXIT_FAILURE);
    }
    ps->route[ps->n_routes] = *pr;
    ps->n_routes++;
}

int main(void) {

    // init server
    PlintServer ps = {0};

    // add routes
    PlintServer_append_route(
            &ps, &(PlintRoute){.route = "/", .file_path = "./files/index.html"});

    // specify address
    ServerAddressIPv4 server_addr = {.ip = {127, 0, 0, 1}, .port = 6969};

    // start server
    Plint_server_start(&ps, server_addr);

    return EXIT_SUCCESS;
}

// helper: Plint_server_start(PlintServer *ps, const ServerAddressIPv4 saddr)
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

char *_Plint_mime_type_get(char *file_path) {
    char *ext = strrchr(file_path, '.');
    if (!ext)
        return "text/plain";
    if (strcmp(ext, ".html") == 0)
        return "text/html";
    if (strcmp(ext, ".css") == 0)
        return "text/css";
    if (strcmp(ext, ".js") == 0)
        return "application/javascript";
    if (strcmp(ext, ".json") == 0)
        return "application/json";
    if (strcmp(ext, ".png") == 0)
        return "image/png";
    if (strcmp(ext, ".jpg") == 0)
        return "image/jpeg";
    if (strcmp(ext, ".gif") == 0)
        return "image/gif";
    return "text/plain";
}

void _Plint_file_serve(int client_socket, char *file_path) {
    int file = open(file_path, O_RDONLY);

    if (file < 0) {
        // TODO: handle_route_not_found(client_socket);
        return;
    }

    struct stat file_stat;
    fstat(file, &file_stat);

#define BUFF_SZ 1024
    char response_header[BUFF_SZ];
    snprintf(
            response_header, BUFF_SZ,
            "HTTP/1.1 200 OK\r\nContent-Length: %lld\r\nContent-Type: %s\r\n\r\n",
            file_stat.st_size, _Plint_mime_type_get(file_path));
    send(client_socket, response_header, strlen(response_header), 0);

    char file_buffer[BUFF_SZ];
    ssize_t bytes_read;

    while ((bytes_read = read(file, file_buffer, BUFF_SZ)) > 0) {
        send(client_socket, file_buffer, (size_t)bytes_read, 0);
    }

    close(file);
}

void _Plint_route_handle(PlintServer *ps, int socket, char *path) {
    for (uint i = 0; i < ps->n_routes; i++) {
        if (strcmp(path, ps->route[i].route) == 0) {
            _Plint_file_serve(socket, ps->route[i].file_path);
            return;
        }
    }
    // TODO: handle_route_not_found(client_socket);
}

#define _PLINT_RECV_BUF_SZ 512
#define _PLINT_HDR_FIELD_SZ 96
void _Plint_client_handle(PlintServer *ps, int socket) {

    char method[_PLINT_HDR_FIELD_SZ] = {0}, path[_PLINT_HDR_FIELD_SZ] = {0},
         http_v[_PLINT_HDR_FIELD_SZ] = {0};
    {
        char buf[_PLINT_RECV_BUF_SZ] = {0};
        ssize_t len = recv(socket, buf, _PLINT_RECV_BUF_SZ, MSG_PEEK);
        if (len < 0) {
            perror("recv");
            exit(EXIT_FAILURE);
        }
        char *ptr = &*buf;
        uint n_len = 0;
        while (*ptr++ != '\r') {
            n_len++;
        }
        buf[n_len] = '\0';
        sscanf(buf, "%s %s %s", method, path, http_v);
    }

    printf("%s(%s) - %s\n", method, http_v, path);

    if (strcmp(http_v, "1.1")) {
        if (strcmp(method, "GET") == 0) {
            _Plint_route_handle(ps, socket, path);
            // char *msg = "200";
            // send(socket, msg, strlen(msg), 0);
        } else if (strcmp(method, "POST") == 0) {
            char *msg = "nice try hacker!";
            send(socket, msg, strlen(msg), 0);
        } else {
            char *msg = "404";
            send(socket, msg, strlen(msg), 0);
            // handle_route_not_found(client_socket);
        }
    }

    shutdown(socket, SHUT_RD);
    close(socket);
}

void Plint_server_start(PlintServer *ps, const ServerAddressIPv4 saddr) {
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
    if (listen(server_fd, LISTEN_BACKLOG) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    ps->server_fd = server_fd;

#ifndef PLINT_NO_LOG
    printf("[LOG] listening on http://%d.%d.%d.%d:%d\n", saddr.ip[0], saddr.ip[1],
            saddr.ip[2], saddr.ip[3], saddr.port);
#endif

    // start running infinitely
    int new_socket;
    struct sockaddr_in peer_addr = {0};
    socklen_t peer_addr_size = sizeof(peer_addr);
    while ((new_socket = accept(ps->server_fd, (struct sockaddr *)&peer_addr,
                    &peer_addr_size))) {
        _Plint_client_handle(ps, new_socket);
    };

    if (new_socket < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }
}
