#include "main.h"

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

/*

   ----> Features
   DONE - "include" (e.g include html files as variables into other html files)

   - variable embedding
   - conditionals
   - loops (over an array)

   things that would be cool to support in the future:
   - layouts (https://axum.code-maven.com/askama-layout.html)

   ----> UX

   The user defines a hashtable of variables (before adding routes) that can
   then be accessed by whatever files the routes point to.

   When appending a new route, do template steps right there to verify
   that everything looks good before the server even gets a chance to start,
   exiting with an error.

   The user might want to be able to change the values of the hashtable while
   the server as running - in that case there ought to be a "update_vartable"
   function or something of that sort where the user can query the key they want
   to change, change it, and then this function will re-validate the relevant
   html files.

*/

/*
   ------------------------ PLINT HTML TEMPLATE SYSTEM ------------------------

   > reserved keywords: embed, loop, item, end

   ----

   > embed
   > - embed html content from a path resolved from the directory of program
   execution, at runtime

   {% embed "incl/hello.html" %}

   ----

   > variable
   > - variables are added in code via PlintServer_append_variable()
   > - any variables you add are global, and can therefore be accessed from
   > - anywhere in your html documents (so be cautious with name collisions!)
   > - you can add either single variables(char*,int) or arrays(char*[], int[])

   > - specifying a single variable:
   <div>
   <p>{{ my_variable }} %}</p>
   </div>

   > - specifying an element from an array variable:
   <div>
   <p>{{ my_array 3 }}</p>
   </div>

   ----
   > condition
   > - added in code via PlintServer_append_variable()
   > - will iterate through all items in an array indiscriminately

   {% if "show_blog_section" %}
   <section>Blog posts here</section>
   {% end %}
   ----

   > loop
   > - added in code via PlintServer_append_variable()
   > - will iterate through all items in an array indiscriminately

   <div>
   <h3>Contact</h3>
   {% loop "arr_contact_info" %}
   <p>{% item %}</p>
   {% end %}
   </div>

   ----------------------------------------------------------------------------
   */

#define PLINT_IMPLEMENTATION
#include "../plint.h"

void setup(PlintServer *ps);

int main(void) {

    PlintServer ps = {0};

    setup(&ps);

    ServerAddressIPv4 server_addr = {.ip = {127, 0, 0, 1}, .port = 6969};

    PlintServer_start(&ps, server_addr);

    return 0;
}

void setup(PlintServer *ps) {
    PlintServer_append_route(
            ps, &(PlintRoute){.route = "/", .file_path = "files/index.html"});

    PlintServer_append_route(
            ps,
            &(PlintRoute){.route = "/projects", .file_path = "files/projects.html"});

    PlintServer_append_route(ps, &(PlintRoute){.route = "/favicon.ico",
            .file_path = "files/white.ico"});
}
