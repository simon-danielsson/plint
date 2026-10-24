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
    
---
<div id="references"></div>
  
## References
   
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

