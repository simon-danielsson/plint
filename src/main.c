#include "main.h"

#define PLINT_IMPLEMENTATION
#include "../plint.h"

// uncomment this to suppress internal log messages
// #define PLINT_NO_LOG

void setup_routes(PlintServer *ps);
void setup_variables(PlintServer *ps);

int main(void) {

    PlintServer ps = {0};

    setup_variables(&ps);
    setup_routes(&ps);

    ServerAddressIPv4 server_addir = {.ip = {127, 0, 0, 1}, .port = 6969};

    PlintServer_start(&ps, server_addir);

    return 0;
}

void setup_variables(PlintServer *ps) {

    PlintServer_append_variable(ps, &(PlintVariable){.key = "show_contact_in_nav",
            .val_k = BOOL,
            .val.b = true});

    PlintServer_append_variable(
            ps, &(PlintVariable){.key = "show_footer", .val_k = BOOL, .val.b = true});

    PlintServer_append_variable(
            ps, &(PlintVariable){.key = "tagline",
            .val_k = STR,
            .val.s = "Recreational programmer"});

    PlintServer_append_variable(
            ps,
            &(PlintVariable){.key = "me", .val_k = STR, .val.s = "Simon Danielsson"});
}

#define ROOT_DIR "files/"

void setup_routes(PlintServer *ps) {

    PlintServer_append_route_many(ps, ROOT_DIR "images", ".jpg");

    PlintServer_append_route(
            ps, &(PlintRoute){.route = "/", .file_path = ROOT_DIR "index.html"});

    PlintServer_append_route(
            ps, &(PlintRoute){.route = "/projects",
            .file_path = ROOT_DIR "projects.html"});

    PlintServer_append_route(ps,
            &(PlintRoute){.route = "/favicon.ico",
            .file_path = ROOT_DIR "white.ico"});
}
