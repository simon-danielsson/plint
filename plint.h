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
#include <dirent.h>
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
#define _PLINT_BUF_SZ 16000

typedef struct {
  off_t _content_size;
  char _content[_PLINT_BUF_SZ];
  char *route;
  char *file_path;
  char *_mime;
} PlintRoute;

typedef enum {
  VAR_STR,
  VAR_INT,
  VAR_DBL,
  ARRAY,
} PlintVariableKind;

typedef struct PlintVariable PlintVariable;
struct PlintVariable {
  PlintVariableKind kind;
  char *key;
  size_t arr_size;
  union {
    int i; // boolean (true|false) || integer (0..9)
    double d;
    char *s;
    const PlintVariable *arr;
  } val;
};

typedef struct {
  ServerAddressIPv4 addr;
  int server_fd;
  PlintRoute route[_PLINT_MAX_ROUTES];
  uint n_routes;
} PlintServer;

#ifndef PLINT_NO_LOG
#define _PlintLog(...)                                                         \
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
#define _PlintLog(...)
#endif

#define _PlintErr(...)                                                         \
  do {                                                                         \
    char tmp[256] = {0};                                                       \
    snprintf(tmp, 256, __VA_ARGS__);                                           \
    fprintf(stderr, "[ERROR] %s\n", tmp);                                      \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

bool Plint_append_route(PlintServer *ps, PlintRoute *pr);
bool Plint_append_route_many(PlintServer *ps, char *parent_path, char *ext);

void Plint_append_variable(PlintVariable *pv);
void PlintServer_start(PlintServer *ps, const ServerAddressIPv4 saddr);

#endif // _PLINT_DEF

#ifdef PLINT_IMPLEMENTATION

global_var char _Plint_FILENAME[FILENAME_MAX] = {0};

// returns new offset on success (current_pos + chars read)
intern_fn int _Plint_read_file_at(char *buf, size_t offset, const char *path) {
  FILE *fp = fopen(path, "rb");
  if (!fp) {
    fclose(fp);
    _PlintErr("%s: Failed to read included file '%s'", _Plint_FILENAME, path);
  }

  if (offset >= _PLINT_BUF_SZ - 1) {
    fclose(fp);
    _PlintErr("%s: Buffer exceeded while reading included file '%s'",
              _Plint_FILENAME, path);
  }

  int c;
  while ((c = fgetc(fp)) != EOF) {
    if (offset >= _PLINT_BUF_SZ - 1) {
      fclose(fp);
      _PlintErr("%s: Buffer exceeded while reading included file '%s'",
                _Plint_FILENAME, path);
    }
    buf[offset++] = (char)c;
  }

  fclose(fp);
  return (int)offset;
}

#ifndef strdup
static inline char *_plint_strdup(const char *s) {
  size_t len = strlen(s) + 1;
  char *d = malloc(len);
  if (d)
    memcpy(d, s, len);
  return d;
}
#define strdup _plint_strdup
#endif

intern_fn void _Plint_FILENAME_update(const char *new_filename) {
  memset(_Plint_FILENAME, 0, FILENAME_MAX);
  size_t len = strlen(new_filename), i = 0;
  for (; i < strlen(new_filename); i++) {
    if (i >= FILENAME_MAX) {
      _PlintErr("Filename length exceeded: %s", new_filename);
    }
    _Plint_FILENAME[i] = new_filename[i];
  }
  _Plint_FILENAME[len] = '\0';
}

#define _Plint_VARS_MAX 200
global_var PlintVariable *VARIABLES[_Plint_VARS_MAX] = {0};
void Plint_append_variable(PlintVariable *var_to_append) {
  for (size_t i = 0; i < _Plint_VARS_MAX; i++) {
    if (VARIABLES[i] == NULL) {
      VARIABLES[i] = var_to_append;
      return;
    }
  }
  _PlintErr(
      "%s: Maximum variable count exceeded while trying to add variable '%s'",
      _Plint_FILENAME, var_to_append->key);
}

#define MAX_DEPTH 4
global_var char result[_PLINT_BUF_SZ] = {0};
global_var uint result_idx, line = 1;

void _template_engine_process_html_RESET(void) {
  memset(result, 0, _PLINT_BUF_SZ);
  result_idx = 0;
}

intern_fn void VARIABLES_pop_last_added(void) {
  for (size_t i = 0; i < _Plint_VARS_MAX; i++) {
    if (VARIABLES[i] == NULL) {
      if (i == 0) {
        _PlintErr("%s:%d -- attempt to pop from empty variable stack",
                  _Plint_FILENAME, line);
      }
      VARIABLES[i - 1] = NULL;
      return;
    }
  }
}

bool _Plint_found(const char *cont, uint curr_idx, char *found) {
  size_t found_len = strlen(found);
  for (size_t i = 0; i < found_len; i++) {
    if (found[i] != cont[curr_idx + i])
      return false;
  }
  // printf("found: %s\n", found);
  return true;
}

#define found(found) _Plint_found(cont, pos, (found))

#define inc(n)                                                                 \
  do {                                                                         \
    if (pos < cont_len) {                                                      \
      pos += (n);                                                              \
      if (pos < cont_len && cont[pos] == '\n')                                 \
        line++;                                                                \
    } else {                                                                   \
      break;                                                                   \
    }                                                                          \
  } while (0)

#define inc_past(str)                                                          \
  do {                                                                         \
    size_t inc_past_len = strlen((str));                                       \
    while (inc_past_len > 0) {                                                 \
      inc(1);                                                                  \
      inc_past_len--;                                                          \
    }                                                                          \
  } while (0)

#define STMT_PRE "{%"
#define STMT_POST "%}"
#define VAR_PRE "{{"
#define VAR_POST "}}"
#define KEYWORD_IF "if"
#define KEYWORD_NEG_OP "!"
#define KEYWORD_INCLUDE "include"
#define KEYWORD_ENDIF "endif"
#define KEYWORD_ENDFOR "endfor"
#define KEYWORD_FOR "for"
#define KEYWORD_IN "in"
#define HTML_CODE_OPEN "<code"
#define HTML_CODE_CLOSE "</code>"

// recursive
uint process_depth(char *cont, size_t cont_len, uint pos, uint depth) {
  while (pos < cont_len) {

    if (found(HTML_CODE_OPEN)) {
      while (!found(HTML_CODE_CLOSE)) {
        result[result_idx++] = cont[pos];
        inc(1);
      }
      assert(found(HTML_CODE_CLOSE) == true);
      for (size_t i = 0; i < strlen(HTML_CODE_CLOSE); i++) {
        result[result_idx++] = cont[pos];
        inc(1);
      }
      // inc_past(HTML_CODE_CLOSE);
      continue;
    }

    if (found(VAR_PRE)) {
      inc_past(VAR_PRE);
      while (pos < cont_len) {
        while (isspace(cont[pos]))
          inc(1);
        char var[128];
        size_t len = 0;
        char var_idx_str[128];
        size_t var_idx_str_len = 0;
        while (!found(VAR_POST) && !isspace(cont[pos])) {
          if (cont[pos] == '[') {
            inc(1);
            assert(cont[pos] != '[');
            while (cont[pos] != ']') {
              var_idx_str[var_idx_str_len++] = cont[pos];
              inc(1);
            }
            assert(cont[pos] == ']');
          } else {
            var[len++] = cont[pos];
          }
          inc(1);
        }
        var[len] = '\0';
        assert(len < sizeof(var));

        int var_idx = -1;
        var_idx_str[var_idx_str_len + 1] = '\0';
        if (var_idx_str[0] != '\0') {
          var_idx = atoi(var_idx_str);
          // printf("var_idx: %d\n", var_idx);
        }

        bool var_was_Plint_found = false;
        for (size_t i = 0; VARIABLES[i] != NULL; i++) {
          if (strcmp(var, VARIABLES[i]->key) == 0) {
            if (VARIABLES[i]->kind == ARRAY) {

              if (var_idx == -1) {
                _PlintErr("%s:%d -- index for array variable '%s' is "
                          "undefined or malformed",
                          _Plint_FILENAME, line, var);
              }

              if ((size_t)var_idx >= VARIABLES[i]->arr_size) {
                _PlintErr("%s:%d -- specified index for array variable "
                          "'%s' is larger than array size",
                          _Plint_FILENAME, line, var);
              }

              PlintVariableKind var_kind = VARIABLES[i]->val.arr[var_idx].kind;

              if (var_kind == VAR_INT) {
                printf("array var kind int\n");
                char tmp[10];
                snprintf(tmp, 10, "%d", VARIABLES[i]->val.arr[var_idx].val.i);
                tmp[9] = '\0';
                for (size_t j = 0; j < strlen(tmp); j++) {
                  result[result_idx++] = tmp[j];
                }

              } else if (var_kind == VAR_DBL) {
                printf("array var kind double\n");
                char tmp[10 + 1];
                snprintf(tmp, 10, "%.2f", VARIABLES[i]->val.arr[var_idx].val.d);
                tmp[10] = '\0';
                for (size_t j = 0; j < strlen(tmp); j++) {
                  result[result_idx++] = tmp[j];
                }
              } else {
                printf("array var kind string\n");
                if (VARIABLES[i]->val.arr[var_idx].val.s == NULL) {
                  _PlintErr("%s:%d -- array variable "
                            "'%s' at index '%d' contains a null value",
                            _Plint_FILENAME, line, var, var_idx);
                }
                for (size_t j = 0;
                     j < strlen(VARIABLES[i]->val.arr[var_idx].val.s); j++) {
                  result[result_idx++] =
                      VARIABLES[i]->val.arr[var_idx].val.s[j];
                }
              }

            } else if (VARIABLES[i]->kind == VAR_INT) {
              char tmp[10 + 1];
              snprintf(tmp, 10, "%d", VARIABLES[i]->val.i);
              tmp[10] = '\0';
              for (size_t j = 0; j < strlen(tmp); j++) {
                result[result_idx++] = tmp[j];
              }

            } else if (VARIABLES[i]->kind == VAR_DBL) {
              char tmp[10 + 1];
              snprintf(tmp, 10, "%.2f", VARIABLES[i]->val.d);
              tmp[10] = '\0';
              for (size_t j = 0; j < strlen(tmp); j++) {
                result[result_idx++] = tmp[j];
              }

            } else {
              for (size_t j = 0; j < strlen(VARIABLES[i]->val.s); j++) {
                result[result_idx++] = VARIABLES[i]->val.s[j];
              }
            }
            var_was_Plint_found = true;
            break;
          }
        }

        if (!var_was_Plint_found) {
          _PlintErr("%s:%d -- variable '%s' is not defined", _Plint_FILENAME,
                    line, var);
        }

        // printf("VAR (l:%d d:%d) -- '%s'\n", line, depth, var);
        int overflow_counter = 5;
        while (!found(VAR_POST) && overflow_counter > 0) {
          inc(1);
          overflow_counter--;
        }

        if (!found(VAR_POST)) {
          _PlintErr("%s:%d -- variable '%s' was not "
                    "escaped properly (missing or malformed '}}')",
                    _Plint_FILENAME, line, var);
        }
        inc_past(VAR_POST);
        break;
      }
    }

    if (found(STMT_PRE)) {
      inc_past(STMT_PRE);
      while (isspace(cont[pos]))
        inc(1);

      if (found(KEYWORD_IF)) {
        inc_past(KEYWORD_IF);
        while (pos < cont_len) {
          while (isspace(cont[pos]))
            inc(1);
          char var[128];
          size_t len = 0;
          bool invert = false;
          while (!found(STMT_POST) && !isspace(cont[pos])) {
            if (cont[pos] == '!') {
              invert = true;
              inc(1);
            }
            var[len++] = cont[pos];
            inc(1);
          }
          while (isspace(cont[pos]))
            inc(1);
          inc_past(STMT_POST);

          var[len] = '\0';
          int var_is_true = -1;

          for (size_t i = 0; VARIABLES[i] != NULL; i++) {
            if (strcmp(var, VARIABLES[i]->key) == 0) {
              var_is_true = VARIABLES[i]->val.i;
              break;
            }
          }

          if (var_is_true < 0) {
            _PlintErr("%s:%d -- boolean '%s' is not defined", _Plint_FILENAME,
                      line, var);
          }

          var_is_true = invert ? !var_is_true : var_is_true;

          // printf("IF-VAR (l:%d d:%d) -- '%s'\n", line, depth, var);
          size_t depth_counter = 1;
          size_t scan = pos;
          size_t endif_start = 0;
          while (scan < cont_len && depth_counter > 0) {
            if (_Plint_found(cont, scan, STMT_PRE)) {
              size_t temp_scan = scan + 2;
              while (temp_scan < cont_len && isspace(cont[temp_scan]))
                temp_scan++;
              if (_Plint_found(cont, temp_scan, KEYWORD_IF) ||
                  _Plint_found(cont, temp_scan, KEYWORD_FOR))
                depth_counter++;
              else if (_Plint_found(cont, temp_scan, KEYWORD_ENDIF) ||
                       _Plint_found(cont, temp_scan, KEYWORD_ENDFOR)) {
                if (--depth_counter == 0) {
                  endif_start = scan;
                  break;
                }
              }
            }
            scan++;
          }

          size_t body_len = endif_start - pos;

          if (var_is_true) {
            pos = process_depth(&cont[pos], body_len, 0, depth + 1);
          } else {
            pos = endif_start;
          }

          size_t temp = endif_start + 2;
          while (temp < cont_len && isspace(cont[temp]))
            temp++;
          while (temp < cont_len && !_Plint_found(cont, temp, STMT_POST))
            temp++;
          temp += 2;
          pos = temp;
          // printf("new pos: %d\n", pos);
          break;
        }
      } else if (found(KEYWORD_INCLUDE)) {
        inc_past(KEYWORD_INCLUDE);
        while (pos < cont_len) {
          while (isspace(cont[pos]))
            inc(1);
          char var[FILENAME_MAX];
          size_t var_len = 0;
          while (!found(STMT_POST)) {
            if (isspace(cont[pos]))
              break;
            else
              var[var_len++] = cont[pos];
            inc(1);
          }
          while (isspace(cont[pos]))
            inc(1);
          assert(cont[pos] == '%');
          inc_past(STMT_POST);
          var[var_len] = '\0';

          _PlintLog("%s:%d -- including '%s'", _Plint_FILENAME, line, var);
          char include_buf[_PLINT_BUF_SZ] = {0};
          int bytes_read = _Plint_read_file_at(include_buf, 0, var);
          process_depth(include_buf, (size_t)bytes_read, 0, depth);

          break;
        }

      } else if (found(KEYWORD_FOR)) {
        inc_past(KEYWORD_FOR);
        while (isspace(cont[pos]))
          inc(1);
        assert(!isspace(cont[pos]));
        while (pos < cont_len) {
          char iter_var[128], array_var[128];
          size_t iter_var_len = 0, array_var_len = 0;
          while (!isspace(cont[pos])) {
            iter_var[iter_var_len++] = cont[pos];
            inc(1);
          }
          iter_var[iter_var_len] = '\0';
          while (isspace(cont[pos]))
            inc(1);
          inc_past(KEYWORD_IN);
          while (isspace(cont[pos]))
            inc(1);
          while (!isspace(cont[pos])) {
            array_var[array_var_len++] = cont[pos];
            inc(1);
          }
          array_var[array_var_len] = '\0';
          inc(1);
          inc_past(STMT_POST);
          // printf("iterable: '%s' - array: '%s'\n", iter_var, array_var);

          bool array_undefined = true;
          for (size_t i = 0; VARIABLES[i] != NULL; i++) {
            if (strcmp(array_var, VARIABLES[i]->key) == 0) {
              array_undefined = false;

              size_t depth_counter = 1;
              size_t scan = pos;
              size_t endfor_start = 0;
              while (scan < cont_len && depth_counter > 0) {
                if (_Plint_found(cont, scan, STMT_PRE)) {
                  size_t temp_scan = scan + 2;
                  while (temp_scan < cont_len && isspace(cont[temp_scan]))
                    temp_scan++;
                  if (_Plint_found(cont, temp_scan, KEYWORD_IF) ||
                      _Plint_found(cont, temp_scan, KEYWORD_FOR))
                    depth_counter++;
                  else if (_Plint_found(cont, temp_scan, KEYWORD_ENDIF) ||
                           _Plint_found(cont, temp_scan, KEYWORD_ENDFOR)) {
                    if (--depth_counter == 0) {
                      endfor_start = scan;
                      break;
                    }
                  }
                }
                scan++;
              }

              size_t body_len = endfor_start - pos;

              for (size_t j = 0; j < VARIABLES[i]->arr_size; j++) {
                Plint_append_variable(&(PlintVariable){
                    .arr_size = VARIABLES[i]->val.arr[j].arr_size,
                    .key = iter_var,
                    .kind = VARIABLES[i]->val.arr[j].kind,
                    .val = VARIABLES[i]->val.arr[j].val});
                process_depth(&cont[pos], body_len, 0, depth + 1);
                VARIABLES_pop_last_added();
              }

              size_t temp = endfor_start + 2;
              while (temp < cont_len && isspace(cont[temp]))
                temp++;
              while (temp < cont_len && !_Plint_found(cont, temp, STMT_POST))
                temp++;
              temp += 2;
              pos = temp;

              break;
            }
          }

          if (array_undefined) {
            _PlintErr("%s:%d -- array '%s' is undefined or malformed",
                      _Plint_FILENAME, line, array_var);
          }

          break;
        }
      }
    }

    result[result_idx++] = cont[pos];
    inc(1);
  }
  return pos;
}

void template_engine_process_html(char *cont, size_t cont_len,
                                  char *new_filename) {
  _Plint_FILENAME_update(new_filename);
  _template_engine_process_html_RESET();

  uint depth = 0, pos = 0;
  process_depth(cont, cont_len, pos, depth);

  // printf("\n-------------\n");
  // printf("%s", result);
  // printf("\n-------------\n");

  assert(depth == 0);
}

// TODO: the method of getting the extension could potentially be buggy
// if there ever was a file called "document.bak.html" or similar?
intern_fn char *_Plint_mime_type_get(char *file_path) {
  char *ext = strrchr(file_path, '.');
  if (!ext)
    return "text/plain";
  if (strcmp(ext, ".html") == 0)
    return "text/html";
  if (strcmp(ext, ".ico") == 0)
    return "image/x-icon";
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

bool Plint_append_route_many(PlintServer *ps, char *parent_path, char *ext) {

  const size_t found_capacity = 128;
  char *found[found_capacity];
  size_t found_count = 0;

  struct dirent *de;
  DIR *dir = opendir(parent_path);

  if (!dir) {
    _PlintErr("Failed to open route '%s'", parent_path);
  }

  while ((de = readdir(dir)) != NULL) {
    if (!ext) {
      found[found_count++] = de->d_name;
    } else if (strstr(de->d_name, ext)) {
      found[found_count++] = de->d_name;
    }
  }

  char tmp_full_path[128];
  char tmp_route[128];

  for (size_t i = 0; i < found_count; i++) {
    memset(tmp_full_path, 0, 128);
    snprintf(tmp_full_path, 128, "%s/%s", parent_path, found[i]);

    memset(tmp_route, 0, 128);
    snprintf(tmp_route, 128, "/%s", found[i]);

    Plint_append_route(
        ps, &(PlintRoute){.route = tmp_route, .file_path = tmp_full_path});
  }

  closedir(dir);

  return true;
}

bool Plint_append_route(PlintServer *ps, PlintRoute *pr) {
  if (ps->n_routes + 1 > _PLINT_MAX_ROUTES) {
    _PlintErr("Number of routes exceeded: %d", _PLINT_MAX_ROUTES);
  }
  if (!pr->file_path) {
    _PlintErr("Path not found: the path of route '%s' is null", pr->route);
  }
  _PlintLog("appending route '%s' with path '%s'", pr->route, pr->file_path);

  char *route_copy = strdup(pr->route);
  char *file_path_copy = strdup(pr->file_path);

  ps->route[ps->n_routes].route = route_copy;
  ps->route[ps->n_routes].file_path = file_path_copy;
  ps->route[ps->n_routes]._mime = _Plint_mime_type_get(file_path_copy);

  // process html template
  if (strstr(ps->route[ps->n_routes]._mime, "html")) {

    int fs = open(pr->file_path, O_RDONLY);
    if (fs < 0) {
      _PlintErr("Path not found: %s", pr->file_path);
    }
    close(fs);

    char buf[_PLINT_BUF_SZ] = {0};
    _Plint_read_file_at(buf, 0, pr->file_path);
    template_engine_process_html(buf, strlen(buf), pr->file_path);

    strcpy(ps->route[ps->n_routes]._content, result);
    ps->route[ps->n_routes]._content_size = (off_t)strlen(result);
  }
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

  // check if there is an html route among the routes that matches request.
  // if that is the case, handle that separately from other mime types
  for (uint i = 0; i < ps->n_routes; i++) {
    if (strcmp(file_path, ps->route[i].file_path) == 0 &&
        strstr(ps->route[i]._mime, "html")) {

      char response_header[_PLINT_BUF_SZ];
      snprintf(response_header, _PLINT_BUF_SZ,
               "HTTP/1.1 200 OK\r\nContent-Length: %lld\r\nContent-Type: "
               "%s\r\n\r\n",
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
    _PlintLog("file not found: %s", file_path);
    // TODO: handle_route_not_Plint_found(client_socket);
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
  ssize_t bytes_read, total_sent = 0;
  _PlintLog("Content-Length: %lld, file: %s", (long long)resp_size, file_path);
  for (;;) {
    bytes_read = read(fs, file_buffer, _PLINT_BUF_SZ);
    if (bytes_read < 0) {
      if (errno == EINTR)
        continue;
      perror("read");
      break;
    }
    if (bytes_read == 0)
      break;
    size_t remaining = (size_t)bytes_read;
    char *buf_ptr = file_buffer;
    ssize_t n;

    while (remaining > 0) {
      n = send(client_socket, buf_ptr, remaining, 0);
      if (n < 0) {
        if (errno == EINTR)
          continue;
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
          continue; // socket temp. unavail.
        }
        perror("send");
        break;
      }
      buf_ptr += n;
      total_sent += n;
      remaining -= (size_t)n;
    }
  }
  _PlintLog("total bytes sent: %zu", total_sent);
  close(fs);
}

intern_fn void _Plint_route_handle(PlintServer *ps, int socket, char *path) {
  for (uint i = 0; i < ps->n_routes; i++) {
    if (strcmp(path, ps->route[i].route) == 0) {
      _Plint_file_serve(ps, socket, ps->route[i].file_path);
      return;
    }
  }
  _PlintLog("route not found: %s", path);
  // TODO: handle_route_not_Plint_found(client_socket);
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

  _PlintLog("%s %s", method, path);

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
      // handle_route_not_Plint_found(client_socket);
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

  _PlintLog("listening on http://%d.%d.%d.%d:%d", saddr.ip[0], saddr.ip[1],
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
