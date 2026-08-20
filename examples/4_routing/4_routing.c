#define PLINT_IMPLEMENTATION
#include "../../plint.h"

int main(void) {
    PlintServer ps = {0};

    Plint_append_route_many(&ps, "images", ".webp");

    Plint_append_route(&ps,
            &(PlintRoute){.route = "/", .file_path = "index.html"});

    Plint_append_route(&ps, &(PlintRoute){.route = "/favicon.ico",
            .file_path = "../../media/plint.ico"});

    ServerAddressIPv4 server_addir = {.ip = {127, 0, 0, 1}, .port = 6969};

    PlintServer_start(&ps, server_addir);

    return 0;
}
