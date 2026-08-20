<p align="center">
    <img src="media/logo.png" alt="plint" width="110"/>
</p>
  
<p align="center">
  <em>Header-only C library for building basic static websites.</em>
</p>

<p align="center">
    <img src="https://img.shields.io/badge/license-MIT-green?style=flat-square" alt="MIT License" />
  <img src="https://img.shields.io/github/last-commit/simon-danielsson/plint/main?style=flat-square&color=blue" alt="Last commit" />
      <img src="https://img.shields.io/badge/C_version-99-cyan?style=flat-square" alt="C" />
</p>
  
<p align="center">
  <a href="#info">Info</a> •
  <a href="#install">Install</a> •
  <a href="#usage">Usage</a>
  <br>
  <a href="#references">References</a> •
  <a href="#license">License</a>
</p>  
  
---
<div id="info"></div>

## Info
  
Plint is (at its core) a wrapper around the Unix networking API; it's adequate for projects such as static websites with low traffic. For deployment, just slap an [nginx](https://nginx.org/en/) reverse proxy on top of it.
  
### Features
    
- HTML template engine
- Serving of static files
- Easy-to-understand error messages
- Bulk asset routing
  
### Planned features
  
- POST request handling
- Customizable 404 page (fallback route)
  
### Requirements
- C99 or newer
- Unix OS
  
---
<div id="install"></div>

## Install
  
Simply clone or copy-paste [plint.h](./plint.h) into your codebase and include it like so:
  
``` c
#define PLINT_IMPLEMENTATION
#include "plint.h"
```
  
---
<div id="usage"></div>
  
## Usage
   
See [USAGE.md](./USAGE.md) for an in-depth walkthrough of Plint.
See [/examples](./examples) to view practical examples of typical usage.
   
### Simple example
    
``` c
#define PLINT_IMPLEMENTATION
#include "../plint.h"

void setup_routes(PlintServer *ps);
void setup_variables(void);

int main(void) {

    PlintServer ps = {0};

    setup_variables();
    setup_routes(&ps);

    ServerAddressIPv4 server_addir = {.ip = {127, 0, 0, 1}, .port = 6969};

    PlintServer_start(&ps, server_addir);

    return 0;
}

void setup_variables(void) {
    static PlintVariable show_contact_in_nav = {
        .key = "show_contact_in_nav", .kind = VAR_INT, .val.i = true};

    static PlintVariable show_footer = {
        .key = "show_footer", .kind = VAR_INT, .val.i = true};

    static PlintVariable tagline = {
        .key = "tagline", .kind = VAR_STR, .val.s = "Recreational programmer"};

    static PlintVariable me = {
        .key = "me", .kind = VAR_STR, .val.s = "Simon Danielsson"};

    Plint_append_variable(&show_contact_in_nav);
    Plint_append_variable(&show_footer);
    Plint_append_variable(&tagline);
    Plint_append_variable(&me);
}

#define ROOT_DIR "files/"

void setup_routes(PlintServer *ps) {

    Plint_append_route_many(ps, ROOT_DIR "images", ".jpg");

    Plint_append_route(
            ps, &(PlintRoute){.route = "/", .file_path = ROOT_DIR "index.html"});

    Plint_append_route(ps, &(PlintRoute){.route = "/projects",
            .file_path = ROOT_DIR "projects.html"});

    Plint_append_route(ps, &(PlintRoute){.route = "/favicon.ico",
            .file_path = ROOT_DIR "white.ico"});
}
```
    
---
<div id="references"></div>
  
## References
   
I have decided to share all the references I studied while building this library, since other people
might find them useful as learning resources.   
   
- https://aosabook.org/en/500L/a-simple-web-server.html
- https://ruslanspivak.com/lsbaws-part1/
- https://joaoventura.net/blog/2017/python-webserver/
- https://github.com/farhaanaliii/cerver
- https://github.com/KDesp73/webc
- https://www.tutorialspoint.com/http/http_api_design_considerations.htm
- https://en.wikibooks.org/wiki/C_programming/Networking_in_UNIX
- https://en.cppreference.com/c/language/compound_literal
- https://github.com/rexim/tore/
- https://github.com/pallets/jinja/
- https://github.com/sqlalchemy/mako
- https://sourceware.org/glibc/manual/latest/html_mono/libc.html
    
---
<div id="license"></div>
  
## License
  
This project is licensed under the [MIT License](https://github.com/simon-danielsson/plint/blob/main/LICENSE).  

