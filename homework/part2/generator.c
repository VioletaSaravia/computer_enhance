#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    int count = argc > 1 ? atoi(argv[1]) : 100;
    char *path = argc > 2 ? argv[2] : "input_uniform.json";

    FILE *f = fopen(path, "w");
    if (f == 0)
        abort();

    fprintf(f, "{\n\t\"pairs\": [\n");

    for (size_t i = 0; i < count; i++)
    {
        float x0 = (float)rand() / (float)(RAND_MAX / 100) - 50.0f;
        float y0 = (float)rand() / (float)(RAND_MAX / 100) - 50.0f;
        float x1 = (float)rand() / (float)(RAND_MAX / 100) - 50.0f;
        float y1 = (float)rand() / (float)(RAND_MAX / 100) - 50.0f;

        fprintf(f, "\t\t{\n");
        fprintf(f, "\t\t\t\"x0\": %.4f,\n", x0);
        fprintf(f, "\t\t\t\"y0\": %.4f,\n", y0);
        fprintf(f, "\t\t\t\"x1\": %.4f,\n", x1);
        fprintf(f, "\t\t\t\"y1\": %.4f\n", y1);
        fprintf(f, "\t\t}");
        if (i + 1 < count)
            fprintf(f, ",");
        fprintf(f, "\n");
    }

    fprintf(f, "\t]\n}");
    fclose(f);
}