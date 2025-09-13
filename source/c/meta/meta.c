#include "gen_array.h"
#include "parser.h"

cstr Basepath(cstr path) {
    static char buf[4096]; // adjust as needed
    cstr        slash = strrchr(path, '/');
#ifdef _WIN32
    cstr bslash = strrchr(path, '\\');
    if (!slash || (bslash && bslash > slash)) slash = bslash;
#endif

    if (!slash) {
        return ".";
    }

    size_t len = (size_t)(slash - path);
    if (len == 0) {
        // path starts with a slash, e.g. "/foo"
        len = 1;
    }

    if (len >= sizeof(buf)) len = sizeof(buf) - 1;
    memcpy(buf, path, len);
    buf[len] = '\0';
    return buf;
}

cstr Filename(cstr path) {
    cstr slash = strrchr(path, '/');
#ifdef _WIN32
    cstr bslash = strrchr(path, '\\');
    if (!slash || (bslash && bslash > slash)) slash = bslash;
#endif
    return slash ? slash + 1 : path;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s file.c\n", argv[0]);
        return 1;
    }

    FILE* f = fopen(argv[1], "rb");

    if (!f) {
        perror("fopen");
        return 1;
    }
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* buf = malloc(len + 1);
    fread(buf, 1, len, f);
    buf[len] = 0;
    fclose(f);

    src      = buf;
    filename = argv[1];
    parse();

    FILE* a = fopen("array_gen.h", "w");
    fprintf(a, "#pragma once\n\n");
    fprintf(a, "#include \"lib/types.h\"\n\n");
    for (int i = 0; i < struct_count; i++) {
        struct StructDef* s = &structs[i];

        if (SDL_strstr(s->gen, "array")) WriteArray(a, s);

        printf("Struct %s", s->name);
        if (s->gen[0]) printf(" (%s)", s->gen);
        printf("\n");
        for (int j = 0; j < s->field_count; j++) {
            struct Field* f = &s->fields[j];
            printf("  %s %s", f->type, f->name);
            if (f->gen[0]) printf(" (%s)", f->gen);
            printf("\n");
        }
    }

    fclose(a);

    free(buf);
    return 0;
}
