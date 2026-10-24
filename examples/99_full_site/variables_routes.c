#include "../../plint.h"

#define SOCIAL_ICON(icon, url)                                                 \
    (PlintVariable) {                                                            \
        .kind = ARRAY, .arr_size = 2, .val.arr = (const PlintVariable[]) {         \
            {.kind = VAR_STR, .val.s = (icon)}, {.kind = VAR_STR, .val.s = (url)},   \
        }                                                                          \
    }

static PlintVariable social_icons = {
    .key = "social_icons",
    .kind = ARRAY,
    .arr_size = 3,
    .val.arr =
        (const PlintVariable[]){
            SOCIAL_ICON("", "https://github.com/simon-danielsson/plint"),
            SOCIAL_ICON(
                    "󰿃",
                    "https://github.com/simon-danielsson/plint/blob/main/LICENSE"),
            SOCIAL_ICON("", "mailto:this_is_not_a_real_email@plint.com"),
        },
};

static PlintVariable site_title = {
    .kind = VAR_STR, .key = "site_title", .val.s = "Plint"};

static PlintVariable description = {
    .kind = VAR_STR,
    .key = "description",
    .val.s = "This is the second example that showcases the use of variables."};

void append_vars(void) {
    Plint_append_variable(&site_title);
    Plint_append_variable(&description);
    Plint_append_variable(&social_icons);

    Plint_append_variable_markdown("references", "static/md/references.md");
}

// routes ---------------------------------------------------------------------

void append_routes(PlintServer *ps) {
    Plint_append_route_many(ps, "../../media", ".png");
    Plint_append_route_many(ps, "static/images", ".webp");
    Plint_append_route_many(ps, "static/fonts", ".ttf");
    Plint_append_route_many(ps, "static/css", ".css");
    Plint_append_route(
            ps, &(PlintRoute){.route = "/", .file_path = "static/index.html"});
    Plint_append_route(ps, &(PlintRoute){.route = "/references",
            .file_path = "static/references.html"});
    Plint_append_route(ps, &(PlintRoute){.route = "/favicon.ico",
            .file_path = "../../media/plint.ico"});
}
