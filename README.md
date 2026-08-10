<p align="center">
    <h2>Plint</h2>
</p>
  
<p align="center">
  <em>Header-only C library for building basic static websites.</em>
</p>

<p align="center">
    <img src="https://img.shields.io/badge/license-MIT-green?style=flat-square" alt="MIT License" />
  <img src="https://img.shields.io/github/last-commit/simon-danielsson/ani/main?style=flat-square&color=blue" alt="Last commit" />
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
  
Plint can essentially be described as a fancy wrapper around the Unix networking API, making the handling of requests and the serving of static web content trivial. This library is adequate for projects such as static websites with low traffic.
  
### Requirements
- C99 or newer
- Unix OS
  
---
<div id="install"></div>

## Install
  
Download (or copy) [plint.h](./plint.h) into your codebase and include it like this:
  
``` c
#define PLT_IMPLEMENTATION
#include "plint.h"
```
  
---
<div id="usage"></div>
  
## Usage
   
No usage section written yet.
    
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
- https://github.com/KDesp73/webc
- https://www.tutorialspoint.com/http/http_api_design_considerations.htm
- https://en.wikibooks.org/wiki/C_programming/Networking_in_UNIX
    
---
<div id="license"></div>
  
## License
  
This project is licensed under the [MIT License](https://github.com/simon-danielsson/ani/blob/main/LICENSE).  

