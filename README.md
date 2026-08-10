<h1 align="center">Plint</h1>
  
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
  
Plint is (at its core) a wrapper around the Unix networking API; it's adequate for projects such as static websites with low traffic.
  
### Features
- HTTP/1.1 support
- Serves static files

### Planned features
- Handling of POST requests
- HTML template system
  
### Requirements
- C99 or newer
- Unix OS
  
---
<div id="install"></div>

## Install
  
Download (or copy) [plint.h](./plint.h) into your codebase and include it like this:
  
``` c
#define PLINT_IMPLEMENTATION
#include "plint.h"
```
  
---
<div id="usage"></div>
  
## Usage
   
### Simple example
    
``` c
#define PLINT_IMPLEMENTATION
#include "plint.h"

int main(void) {
    PlintServer ps = {0};

    PlintServer_append_route(
            &ps, &(PlintRoute){.route = "/", .file_path = "./site/index.html"});

    PlintServer_append_route(
            &ps,
            &(PlintRoute){.route = "/projects", .file_path = "./site/projects.html"});

    PlintServer_append_route(&ps, &(PlintRoute){.route = "/favicon.ico",
            .file_path = "./site/white.ico"});

    ServerAddressIPv4 server_addr = {.ip = {127, 0, 0, 1}, .port = 6969};

    PlintServer_start(&ps, server_addr);

    return 0;
}
```
    
---
<div id="references"></div>
  
## References
   
I have decided to share all the references I found while building this library, since other people
might find them useful as learning resources. *Note: it's important to compare any
sources you find on the internet with official and trusted documentation (in the
case of this library, everything regarding UNIX networking can be found on the man pages).*
   
- https://aosabook.org/en/500L/a-simple-web-server.html
- https://ruslanspivak.com/lsbaws-part1/
- https://joaoventura.net/blog/2017/python-webserver/
- https://github.com/farhaanaliii/cerver
- https://github.com/KDesp73/webc
- https://www.tutorialspoint.com/http/http_api_design_considerations.htm
- https://en.wikibooks.org/wiki/C_programming/Networking_in_UNIX
- https://github.com/rexim/tore/
- https://github.com/pallets/jinja/
    
---
<div id="license"></div>
  
## License
  
This project is licensed under the [MIT License](https://github.com/simon-danielsson/plint/blob/main/LICENSE).  

