#define PLINT_IMPLEMENTATION
#include "../../plint.h"

// Note: an array needs to be declared outside of a function so that
// its storage duration is not limited by a function scope.
static PlintVariable favorite_foods = {
    .key = "favorite_foods",
    .kind = ARRAY,
    .arr_size = 4,
    .val.arr =
        (const PlintVariable[]){
            {.kind = VAR_STR, .val.s = "Tuna pizza"},
            {.kind = VAR_STR, .val.s = "Lasagna"},
            {.kind = VAR_STR, .val.s = "Burek"},
            {.kind = VAR_STR, .val.s = "Sushi"},
        },
};

int main(void) {
    PlintServer ps = {0};

    Plint_append_variable(&favorite_foods);

    Plint_append_route(&ps,
            &(PlintRoute){.route = "/", .file_path = "index.html"});

    Plint_append_route(&ps, &(PlintRoute){.route = "/favicon.ico",
            .file_path = "../../media/plint.ico"});

    ServerAddressIPv4 server_addir = {.ip = {127, 0, 0, 1}, .port = 6969};

    PlintServer_start(&ps, server_addir);

    return 0;
}
