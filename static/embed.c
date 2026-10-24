#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define err(...)                                                                   do {                                                                                 fprintf(stderr, "Error: ");                                                        fprintf(stderr, __VA_ARGS__);                                                      fprintf(stderr, "\n");                                                         } while (0)

char *str_dup(const char *s) {
    char *out;
    if (!s)
        return NULL;
    out = malloc(strlen(s) + 1);
    if (!out)
        return NULL;
    strcpy(out, s);
    return out;
}

char *read_file(const char *path, size_t *len) {
    const size_t MAX_SIZE = 1073741824;
    size_t cap = 1024;
    int c;
    char *content;

    FILE *f = fopen(path, "rb");
    if (!f)
        return NULL;

    content = (char *)malloc(sizeof(char) * cap);
    if (!content)
        goto failure;

    while ((c = fgetc(f)) != EOF) {
        if (*len >= cap) {
            cap *= 2, content = (char *)realloc(content, sizeof(char) * cap);
            if (!content)
                goto failure;
        } else if ((sizeof(content) * *len) > MAX_SIZE) {
            break;
        }
        content[(*len)++] = c;
    }
    content[*len] = '\0';

    fclose(f);
    return content;
failure:
    fclose(f);
    return NULL;
}

char *format_name(char *name) {
    #define out_max_len 512
    char out[out_max_len];
    if (!name)
        return NULL;
    size_t len_name = strlen(name), out_len = 0, i = 0;

    if (isdigit(name[0])) {
        if (len_name == 1) {
            strcpy(out, "embedded_");
            out_len += 9, out[out_len++] = name[0], out[out_len] = '\0';
            return str_dup(out);
        }
        out[out_len++] = '_', i = 1;
    }
    if (len_name == 1) {
        out[out_len++] = name[0], out[out_len] = '\0';
        return str_dup(out);
    }

    for (; i < len_name; i++) {
        if ((!isalnum((unsigned char)name[i])) && !isdigit(name[i])) {
            out[out_len++] = '_';
            goto cont;
        }
        out[out_len++] = tolower(name[i]);
cont:
        if (out_len + 1 > out_max_len)
            return NULL;
    }
    out[out_len] = '\0';
    return str_dup(out);
}

typedef struct {
    char *file_name;
    char *content;
    size_t content_len;
} HeaderFile;

void str_upper(char *s) {
    for (char *c = s; *c; c++)
        *c = toupper(*c);
}

bool HeaderFile_write(HeaderFile *hf) {
    unsigned char *c = (unsigned char *)hf->content;
    const int col_max = 10;
    int col = 0;

    FILE *out;
    {
        char header_name[512] = {0};
        snprintf(header_name, 512, "%s.h", hf->file_name);
        out = fopen(header_name, "w");
    }
    if (!out)
        return false;

    fprintf(out, "#ifndef _embedded_%s_h\n", hf->file_name);
    fprintf(out, "#define _embedded_%s_h\n\n", hf->file_name);

    fprintf(out, "unsigned char %s[] = {\n", hf->file_name);
    while (*c) {
        fprintf(out, "0x%02X, ", *c);
        col++, c++;
        if (col >= col_max)
            fprintf(out, "\n"), col = 0;
        if ((*c + 1) == 0)
            break;
    }
    fprintf(out, "0x%02X", ++(*c));
    fprintf(out, "\n};\n\n");
    fprintf(out, "unsigned int %s_len = %zu;", hf->file_name, hf->content_len);
    fprintf(out, "\n\n#endif");
    fclose(out);
    return true;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        err("Error: no arguments were provided!");
        return 1;
    }

    char *filename = argv[1];

    char *filename_formatted;
    if (!(filename_formatted = format_name(filename))) {
        err("failed to format filename '%s'", filename);
        return 1;
    }

    char *content;
    size_t content_len = 0;
    if (!(content = read_file(filename, &content_len))) {
        err("file '%s' not found", filename);
        return 1;
    }

    HeaderFile hf = {.file_name = filename_formatted,
        .content = content,
        .content_len = content_len};

    if (!HeaderFile_write(&hf)) {
        err("file '%s' not found", filename);
        goto crash;
    }

    free(content);
    free(filename_formatted);
    return 0;
crash:
    free(content);
    free(filename_formatted);
    return 1;
}
