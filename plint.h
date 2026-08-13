
/*

-------------------------------------------------------------------------------

Plint - Header-only C library for building basic static websites.

Source:
    https://github.com/simon-danielsson/plint

Author:
    https://simondanielsson.se/
    contact@simondanielsson.se

-------------------------------------------------------------------------------

Copyright © 2026 Simon Danielsson

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files, to deal in the Software
without restriction, including without limitation the rights to use, copy,
modify, merge, publish, distribute, sublicense, and/or sell copies of the
Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

-------------------------------------------------------------------------------
*/

#ifndef _INCLUDE_PLINT_H
#define _INCLUDE_PLINT_H

#ifndef _PLINT_DEF
#define _PLINT_DEF

#include <arpa/inet.h>
#include <assert.h>
#include <ctype.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define global_var static
#define intern_fn static
typedef uint32_t uint;

typedef struct {
  uint8_t ip[4];
  uint16_t port;
} ServerAddressIPv4;

#define _PLINT_MAX_ROUTES 100
#define _PLINT_MAX_VARIABLES 200
#define _PLINT_BUF_SZ 20000

typedef struct {
  off_t _content_size;
  char _content[_PLINT_BUF_SZ];
  char *route;
  char *file_path;
  char *_mime;
} PlintRoute;

enum PlintVariableKind {
  PVK_STR,
  PVK_INT,
  PVK_FLOAT,
  PVK_STR_ARRAY,
  PVK_INT_ARRAY,
  PVK_FLOAT_ARRAY,
};

typedef struct {
  char *key;
  enum PlintVariableKind val_k;
  union {
    char *s;            // DONE
    int i;              // TODO: add branch to _Plint_embed_variable_at();
    float f;            // TODO: add branch to _Plint_embed_variable_at();
    char *s_array[128]; // TODO: add branch to _Plint_embed_variable_at();
    int i_array[128];   // TODO: add branch to _Plint_embed_variable_at();
    float f_array[128]; // TODO: add branch to _Plint_embed_variable_at();
  } val;
} PlintVariable;

typedef struct {
  ServerAddressIPv4 addr;
  int server_fd;
  PlintRoute route[_PLINT_MAX_ROUTES];
  uint n_routes;
  PlintVariable variable[_PLINT_MAX_VARIABLES];
  uint n_variables;
} PlintServer;

#ifndef PLINT_NO_LOG
#define Plint_log(...)                                                         \
  do {                                                                         \
    time_t rawtime;                                                            \
    struct tm *timeinfo;                                                       \
    char tmp[256] = {0}, time_str[256] = {0};                                  \
    time(&rawtime);                                                            \
    timeinfo = localtime(&rawtime);                                            \
    snprintf(time_str, 256, "%04d-%02d-%02d %02d:%02d:%02d",                   \
             timeinfo->tm_year + 1900, timeinfo->tm_mon + 1,                   \
             timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min,           \
             timeinfo->tm_sec);                                                \
    snprintf(tmp, 256, __VA_ARGS__);                                           \
    printf("[%s] %s \n", time_str, tmp);                                       \
  } while (0)
#else
#define Plint_log(...)
#endif

typedef enum {
  _PLT_PATH_NOT_FOUND,
  _PLT_ROUTE_NOT_FOUND,
  _PLT_VAR_NOT_FOUND,
  _PLT_MAX_ROUTES_EXCEED,
  _PLT_MAX_VARS_EXCEED,
  _PLT_FAILED_TO_OPEN,
  _PLT_FAILED_TO_READ,
  _PLT_BUFFER_EXCEED,
  _PLT_PARSE_FAILURE,
} _PlintErrKind;

#define Plint_err(ERR_KIND, ...)                                               \
  do {                                                                         \
    char *msg;                                                                 \
    switch ((ERR_KIND)) {                                                      \
    case _PLT_VAR_NOT_FOUND:                                                   \
      msg = "variable could not be found";                                     \
      break;                                                                   \
    case _PLT_MAX_ROUTES_EXCEED:                                               \
      msg = "max routes exceeded";                                             \
      break;                                                                   \
    case _PLT_MAX_VARS_EXCEED:                                                 \
      msg = "max variables exceeded";                                          \
      break;                                                                   \
    case _PLT_BUFFER_EXCEED:                                                   \
      msg = "buffer exceeded";                                                 \
      break;                                                                   \
    case _PLT_ROUTE_NOT_FOUND:                                                 \
      msg = "route not found";                                                 \
      break;                                                                   \
    case _PLT_PATH_NOT_FOUND:                                                  \
      msg = "path not found";                                                  \
      break;                                                                   \
    case _PLT_FAILED_TO_OPEN:                                                  \
      msg = "failed to open";                                                  \
      break;                                                                   \
    case _PLT_FAILED_TO_READ:                                                  \
      msg = "failed to read";                                                  \
      break;                                                                   \
    case _PLT_PARSE_FAILURE:                                                   \
      msg = "parse failure";                                                   \
      break;                                                                   \
    }                                                                          \
    char tmp[256] = {0};                                                       \
    snprintf(tmp, 256, __VA_ARGS__);                                           \
    printf("[ERROR] %s: '%s' \n", msg, tmp);                                   \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

bool PlintServer_append_route(PlintServer *ps, PlintRoute *pr);
void PlintServer_append_variable(PlintServer *ps, PlintVariable *pv);
void PlintServer_start(PlintServer *ps, const ServerAddressIPv4 saddr);

#endif // _PLINT_DEF

#ifdef PLINT_IMPLEMENTATION

void PlintServer_append_variable(PlintServer *ps, PlintVariable *pv) {
  if (ps->n_variables >= _PLINT_MAX_VARIABLES - 1) {
    Plint_err(_PLT_MAX_VARS_EXCEED, "%s", pv->key);
  }
  ps->variable[ps->n_variables++] = *pv;
}

// returns new offset on success (current_pos + chars read)
intern_fn int _Plint_embed_variable_at(PlintServer *ps, char *buf,
                                       size_t offset, const char *key) {

  if (offset >= _PLINT_BUF_SZ - 1) {
    Plint_err(_PLT_BUFFER_EXCEED, "variable %s", key);
  }

  int idx = -1;
  for (uint i = 0; i < ps->n_variables; i++) {
    if (strcmp(ps->variable[i].key, key) == 0) {
      idx = (int)i;
      break;
    }
  }
  if (idx < 0) {
    Plint_err(_PLT_VAR_NOT_FOUND, "%s", key);
  }

  // Plint_log("variable {%s = %s}", ps->variable[idx].key,
  // ps->variable[idx].val.s);

  char *c = ps->variable[idx].val.s;
  while (*c) {
    if (offset >= _PLINT_BUF_SZ - 1) {
      Plint_err(_PLT_BUFFER_EXCEED, "variable %s", key);
    }
    buf[offset++] = *c;
    c++;
  }

  return (int)offset;
}

// returns new offset on success (current_pos + chars read)
intern_fn int _Plint_read_file_at(char *buf, size_t offset, const char *path) {
  FILE *fp = fopen(path, "rb");
  if (!fp) {
    fclose(fp);
    Plint_err(_PLT_FAILED_TO_OPEN, "include %s", path);
  }

  if (offset >= _PLINT_BUF_SZ - 1) {
    fclose(fp);
    Plint_err(_PLT_BUFFER_EXCEED, "include %s", path);
  }

  int c;
  while ((c = fgetc(fp)) != EOF) {
    if (offset >= _PLINT_BUF_SZ - 1) {
      fclose(fp);
      Plint_err(_PLT_BUFFER_EXCEED, "include %s", path);
    }
    buf[offset++] = (char)c;
  }

  fclose(fp);
  return (int)offset;
}

void _Plint_file_embed_if_include(PlintServer *ps, char *new_cont, char *cont,
                                  char *file) {
#define _plint_upcoming (char[]){cont[i], cont[i + 1], 0}
  // TODO: my parsing error checking could be described as "lazy" at best
#define _plint_file_emb_inc(n, append)                                         \
  do {                                                                         \
    if (i + (n) <= len) {                                                      \
      for (int ci = 0; ci < (n); ci++)                                         \
        if ((append)) {                                                        \
          new_cont[new_cont_len++] = cont[i++];                                \
        } else {                                                               \
          i++;                                                                 \
        }                                                                      \
    } else {                                                                   \
      Plint_err(_PLT_PARSE_FAILURE, "%s", file);                               \
    }                                                                          \
  } while (0)

  uint i = 0, len = strlen(cont);
#define _plint_tmp_sz 64
  char tmp[_plint_tmp_sz] = {0};

  size_t new_cont_len = 0;

  while (i < len) {
    if (i < len - 3) {

      // variable
      if (strcmp(_plint_upcoming, "{{") == 0) {
        memset(tmp, 0, _plint_tmp_sz);
        int j = 0;
        _plint_file_emb_inc(2, false);
        while (cont[i] == ' ') {
          _plint_file_emb_inc(1, false);
        }

        //_plint_file_emb_inc(1, false);
        while (cont[i] != ' ' && cont[i] != '}') {
          tmp[j++] = cont[i];
          _plint_file_emb_inc(1, false);
        }

        while (strcmp(_plint_upcoming, "}}") != 0) {
          _plint_file_emb_inc(1, false);
        }

        size_t out_offset = new_cont_len;

        int new_offset =
            _Plint_embed_variable_at(ps, new_cont, out_offset, tmp);

        if (new_offset <= 0)
          Plint_err(_PLT_PARSE_FAILURE, "%s", file);

        new_cont_len = (size_t)new_offset;

        _plint_file_emb_inc(2, false);
      }

      // include
      if (strcmp(_plint_upcoming, "{%") == 0) {
        memset(tmp, 0, _plint_tmp_sz);
#undef _plint_tmp_sz
        int j = 0;
        _plint_file_emb_inc(2, false);
        while (cont[i] != '"') {
          _plint_file_emb_inc(1, false);
        }

        _plint_file_emb_inc(1, false);
        while (cont[i] != '"') {
          tmp[j++] = cont[i];
          _plint_file_emb_inc(1, false);
        }

        while (strcmp(_plint_upcoming, "%}") != 0) {
          _plint_file_emb_inc(1, false);
        }

        size_t out_offset = new_cont_len;

        int new_offset = _Plint_read_file_at(new_cont, out_offset, tmp);

        if (new_offset <= 0)
          Plint_err(_PLT_PARSE_FAILURE, "%s", file);

        new_cont_len = (size_t)new_offset;

        _plint_file_emb_inc(2, false);
      }
    }
    _plint_file_emb_inc(1, true);
  }
#undef _plint_tmp_sz
  new_cont[new_cont_len] = '\0';
}

// TODO: the method of getting the extension could potentially be buggy
// if there ever was a file called "document.bak.html" or similar?
intern_fn char *_Plint_mime_type_get(char *file_path) {
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

bool PlintServer_append_route(PlintServer *ps, PlintRoute *pr) {
  if (ps->n_routes + 1 > _PLINT_MAX_ROUTES) {
    Plint_err(_PLT_MAX_ROUTES_EXCEED, "%d", _PLINT_MAX_ROUTES);
  }
  Plint_log("appending route: %s", pr->file_path);

  pr->_mime = _Plint_mime_type_get(pr->file_path);

  // process html template
  if (strstr(pr->_mime, "html")) {

    int fs = open(pr->file_path, O_RDONLY);
    if (fs < 0) {
      Plint_err(_PLT_PATH_NOT_FOUND, "%s", pr->file_path);
    }
    close(fs);

    char html[_PLINT_BUF_SZ] = {0};
    char buf[_PLINT_BUF_SZ] = {0};
    _Plint_read_file_at(buf, 0, pr->file_path);
    _Plint_file_embed_if_include(ps, html, buf, pr->file_path);

    strcpy(pr->_content, html);
    pr->_content_size = sizeof(html);
  }

  ps->route[ps->n_routes] = *pr;
  ps->n_routes++;
  return true;
}

// helper: PlintServer_start(PlintServer *ps, const ServerAddressIPv4 saddr)
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

intern_fn void _Plint_file_serve(PlintServer *ps, int client_socket,
                                 char *file_path) {

  // check if there is a html route among the routes that matches request.
  // if that is the case, handle that separately from other mime types
  for (uint i = 0; i < ps->n_routes; i++) {
    if (strcmp(file_path, ps->route[i].file_path) == 0 &&
        strstr(ps->route[i]._mime, "html")) {

      char response_header[_PLINT_BUF_SZ];
      snprintf(
          response_header, _PLINT_BUF_SZ,
          "HTTP/1.1 200 OK\r\nContent-Length: %lld\r\nContent-Type: %s\r\n\r\n",
          ps->route[i]._content_size, ps->route[i]._mime);
      send(client_socket, response_header, strlen(response_header), 0);

      size_t sent = 0;

      while (sent < (size_t)ps->route[i]._content_size) {
        size_t to_send =
            ((size_t)ps->route[i]._content_size - sent) < _PLINT_BUF_SZ
                ? ((size_t)ps->route[i]._content_size - sent)
                : _PLINT_BUF_SZ;

        ssize_t n =
            send(client_socket, ps->route[i]._content + sent, to_send, 0);
        if (n < 0) {
          // TODO: handle error (EAGAIN/EWOULDBLOCK)
          break;
        }
        sent += (size_t)n;
      }
      return;
    }
  }

  int fs = open(file_path, O_RDONLY);
  if (fs < 0) {
    Plint_log("file not found: %s", file_path);
    // TODO: handle_route_not_found(client_socket);
    return;
  }
  struct stat file_stat;
  fstat(fs, &file_stat);
  char *mime = _Plint_mime_type_get(file_path);
  off_t resp_size;

  resp_size = file_stat.st_size;

  char response_header[_PLINT_BUF_SZ];
  snprintf(
      response_header, _PLINT_BUF_SZ,
      "HTTP/1.1 200 OK\r\nContent-Length: %lld\r\nContent-Type: %s\r\n\r\n",
      resp_size, mime);
  send(client_socket, response_header, strlen(response_header), 0);

  char file_buffer[_PLINT_BUF_SZ];
  ssize_t bytes_read;
  while ((bytes_read = read(fs, file_buffer, _PLINT_BUF_SZ)) > 0) {
    send(client_socket, file_buffer, (size_t)bytes_read, 0);
  }

  close(fs);
}

intern_fn void _Plint_route_handle(PlintServer *ps, int socket, char *path) {
  for (uint i = 0; i < ps->n_routes; i++) {
    if (strcmp(path, ps->route[i].route) == 0) {
      _Plint_file_serve(ps, socket, ps->route[i].file_path);
      return;
    }
  }
  Plint_log("route not found: %s", path);
  // TODO: handle_route_not_found(client_socket);
}

#define _PLINT_HDR_FIELD_SZ 96
intern_fn void _Plint_client_handle(PlintServer *ps, int socket) {

  char method[_PLINT_HDR_FIELD_SZ] = {0}, path[_PLINT_HDR_FIELD_SZ] = {0},
       http_v[_PLINT_HDR_FIELD_SZ] = {0};
  {
    char buf[_PLINT_BUF_SZ] = {0};
    ssize_t len = recv(socket, buf, _PLINT_BUF_SZ, MSG_PEEK);
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

  Plint_log("%s %s", method, path);

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
  if (listen(server_fd, LISTEN_BACKLOG) == -1) {
    perror("listen");
    exit(EXIT_FAILURE);
  }

  ps->server_fd = server_fd;

  Plint_log("listening on http://%d.%d.%d.%d:%d", saddr.ip[0], saddr.ip[1],
            saddr.ip[2], saddr.ip[3], saddr.port);

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
#endif // PLINT_IMPLEMENTATION
#endif // _INCLUDE_PLINT_H
