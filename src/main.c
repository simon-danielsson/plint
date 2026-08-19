#include "main.h"

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
