#define PLINT_IMPLEMENTATION
#include "../../plint.h"

int main(void) {
    PlintServer ps = {0};

    static PlintVariable n_example = {
        .kind = VAR_INT, .key = "n_example", .val.i = 2};

    static PlintVariable description = {
        .kind = VAR_STR,
        .key = "description",
        .val.s =
            "This is the second example that showcases the use of variables."};

    Plint_append_variable(&n_example);
    Plint_append_variable(&description);

    Plint_append_route(&ps,
            &(PlintRoute){.route = "/", .file_path = "index.html"});

    Plint_append_route(&ps, &(PlintRoute){.route = "/favicon.ico",
            .file_path = "../../media/plint.ico"});

    ServerAddressIPv4 server_addir = {.ip = {127, 0, 0, 1}, .port = 6969};

    PlintServer_start(&ps, server_addir);

    return 0;
}
