#define PLINT_IMPLEMENTATION
#include "../../plint.h"
#include "site.h"

void append_vars(void);
void append_routes(PlintServer *ps);

// program --------------------------------------------------------------------

void Plint_append_variable_markdown(char *key, const char *md_filepath);

int main(void) {
    PlintServer ps = {0};

    append_vars();

    append_routes(&ps);

    ServerAddressIPv4 server_addir = {.ip = {127, 0, 0, 1}, .port = 6969};

    PlintServer_start(&ps, server_addir);

    return 0;
}
