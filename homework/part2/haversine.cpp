#include "types.h"
#include "profiler.cpp"
#include <stdio.h>
#include <math.h>

Array<u8> ReadEntireFile(const char *path)
{
    Array<u8> result = {};
    FILE *f = {};
    u64 rd = 0;

    REPETITION_PROFILE("ReadEntireFile inner w/ malloc", 10000);

    result.data = (u8 *)malloc((u64)1024 * 1024 * 512);
    fopen_s(&f, path, "rb");
    if (!f)
    {
        goto cleanup;
    }
    if (fseek(f, 0, SEEK_END) != 0)
    {
        goto cleanup;
    }
    result.cap = ftell(f);
    result.len = result.cap;
    // u8* buf = (u8 *)malloc((u64)sz);
    REPETITION_BANDWIDTH(result.cap);
    if (result.cap < 0)
    {
        goto cleanup;
    }
    rewind(f);
    if (!result.data)
    {
        goto cleanup;
    }

    rd = fread(result.data, 1, (u64)result.cap, f);
    fclose(f);
    f = 0;
    if (rd != (u64)result.cap)
    {
        free(result.data);
        return {};
    }

    free(result.data); // REMOVE!!!
    result.data = 0;
    REPETITION_END();
    return result;

cleanup:
    if (f)
        fclose(f);
    if (result.data)
        free(result.data);
    return {};
}

typedef struct
{
    double x0, y0, x1, y1;
} HaversinePair;

static double Haversine(HaversinePair p, double radius)
{
    const double dLat = Deg2Rad(p.y1 - p.y0);
    const double dLon = Deg2Rad(p.x1 - p.x0);
    const double lat1 = Deg2Rad(p.y0);
    const double lat2 = Deg2Rad(p.y1);

    const double a = pow(sin(dLat * 0.5), 2.0) +
                     cos(lat1) * cos(lat2) * pow(sin(dLon * 0.5), 2.0);
    const double c = 2.0 * asin(sqrt(a));
    return radius * c;
}

static double SumHaversines(const HaversinePair *pairs, size_t n)
{
    PROFILE_FUNCTION();
    if (n == 0)
        return 0.0;
    const double coef = 1.0 / (double)n;
    double sum = 0.0;
    for (size_t i = 0; i < n; ++i)
    {
        Profiler::Get().AddBytes(sizeof(HaversinePair));
        sum += coef * Haversine(pairs[i], 6372.8);
    }
    return sum;
}

#define VEC(type)   \
    typedef struct  \
    {               \
        type *data; \
        size_t len; \
        size_t cap; \
    } Vec##type

VEC(HaversinePair);

static void pairvec_reserve(VecHaversinePair *v, size_t want)
{
    if (want <= v->cap)
        return;
    size_t new_cap = v->cap ? v->cap : 16;
    while (new_cap < want)
        new_cap *= 2;
    HaversinePair *nd = (HaversinePair *)realloc(v->data, new_cap * sizeof(*nd));
    if (!nd)
    {
        fprintf(stderr, "alloc fail\n");
        exit(1);
    }
    v->data = nd;
    v->cap = new_cap;
}
static void pairvec_push(VecHaversinePair *v, HaversinePair p)
{
    PROFILE_FUNCTION();
    if (v->len == v->cap)
        pairvec_reserve(v, v->cap ? v->cap * 2 : 16);
    v->data[v->len++] = p;
}

/* ---------- tiny string builder ---------- */
typedef struct
{
    char *buf;
    size_t len;
    size_t cap;
} StrB;

static void strb_reset(StrB *s) { s->len = 0; }
static void strb_reserve(StrB *s, size_t want)
{
    PROFILE_FUNCTION();
    if (want <= s->cap)
        return;
    size_t nc = s->cap ? s->cap : 32;
    while (nc < want)
        nc *= 2;
    char *nb = (char *)realloc(s->buf, nc);
    if (!nb)
    {
        fprintf(stderr, "alloc fail\n");
        exit(1);
    }
    s->buf = nb;
    s->cap = nc;
}
static void strb_pushc(StrB *s, char c)
{
    if (s->len + 1 >= s->cap)
        strb_reserve(s, s->cap ? s->cap * 2 : 32);
    s->buf[s->len++] = c;
}
static const char *strb_cstr(StrB *s)
{
    if (s->len + 1 >= s->cap)
        strb_reserve(s, s->len + 2);
    s->buf[s->len] = '\0';
    return s->buf;
}
static void strb_free(StrB *s) { free(s->buf); }

static double frand_unit(void) { return (double)rand() / (double)RAND_MAX; }

static void GenerateHaversineJson(int count, const char *path)
{
    PROFILE_FUNCTION();

    FILE *f = {};
    fopen_s(&f, path, "wb");
    if (!f)
    {
        fprintf(stderr, "open %s failed\n", path);
        exit(1);
    }

    fputs("[\n", f);
    char buf[64];

    for (int i = 0; i < count; ++i)
    {
        PROFILE_ADD_BANDWIDTH(3 + 6 + 6 + 6 + 6 + 4);
        fputs("\t{\n", f);

        double v = frand_unit() * 160.0 - 80.0;
        snprintf(buf, sizeof(buf), "\t\t\"x0\": %.2f,\n", v);
        fputs(buf, f);

        v = frand_unit() * 160.0 - 80.0;
        snprintf(buf, sizeof(buf), "\t\t\"y0\": %.2f,\n", v);
        fputs(buf, f);

        v = frand_unit() * 160.0 - 80.0;
        snprintf(buf, sizeof(buf), "\t\t\"x1\": %.2f,\n", v);
        fputs(buf, f);

        v = frand_unit() * 160.0 - 80.0;
        snprintf(buf, sizeof(buf), "\t\t\"y1\": %.2f\n", v);
        fputs(buf, f);

        fputs("\t}", f);
        if (i + 1 != count)
            fputc(',', f);
        fputc('\n', f);
    }
    fputs("]\n", f);
    PROFILE_ADD_BANDWIDTH(4);
    fclose(f);
}

typedef enum
{
    PS_OpenBracket,
    PS_OpenBrace,
    PS_KeyStart,
    PS_Key,
    PS_Colon,
    PS_Value,
    PS_Comma,
    PS_CloseBrace
} HaversineParsingState;

typedef struct
{
    const char *key;
    double value;
} JsonKeyValue;

typedef struct
{
    VecHaversinePair result;
    bool success;
} ParseResult;

static ParseResult ParseHaversineJson(Array<u8> &bytes, int expected_count)
{
    PROFILE_FUNCTION();
    ParseResult pr = {};
    if (expected_count > 0)
        pairvec_reserve(&pr.result, (size_t)expected_count + 1);

    HaversineParsingState state = PS_OpenBracket;
    StrB curKey = {}, curVal = {};

    HaversinePair curPair = {};
    JsonKeyValue curKv = {};

    for (size_t i = 0; i < bytes.cap; ++i)
    {
        Profiler::Get().AddBytes(1);
        unsigned char b = bytes.data[i];

        if ((b == '\n' || b == '\t' || b == '\r' || b == ' ') && state != PS_Key)
            continue;

        switch (state)
        {
        case PS_OpenBracket:
        {
            if (b != '[')
                goto done;
            else
                state = PS_OpenBrace;
            break;
        }
        case PS_OpenBrace:
        {
            if (b != '{')
                goto done;
            else
                state = PS_KeyStart;
            break;
        }
        case PS_KeyStart:
        {
            if (b != '\"')
                goto done;
            else
                state = PS_Key;
            break;
        }
        case PS_Key:
        {
            if (b == '\"')
            {
                state = PS_Colon;
            }
            else
            {
                strb_pushc(&curKey, (char)b);
            }
            break;
        }
        case PS_Colon:
        {
            if (b == ':')
            {
                curKv.key = strb_cstr(&curKey);
                state = PS_Value;
            }
            else
            {
                goto done;
            }
            break;
        }
        case PS_Value:
        {
            if (b == ',' || b == '}')
            {
                const char *val = strb_cstr(&curVal);
                curKv.value = strtod(val, NULL);

                if (strcmp(curKv.key, "x0") == 0)
                    curPair.x0 = curKv.value;
                else if (strcmp(curKv.key, "y0") == 0)
                    curPair.y0 = curKv.value;
                else if (strcmp(curKv.key, "x1") == 0)
                    curPair.x1 = curKv.value;
                else if (strcmp(curKv.key, "y1") == 0)
                    curPair.y1 = curKv.value;

                strb_reset(&curKey);
                strb_reset(&curVal);

                state = (b == ',') ? PS_Comma : PS_CloseBrace;
            }
            else
            {
                strb_pushc(&curVal, (char)b);
                continue; // stay in Value
            }
            break;
        }
        case PS_Comma:
        {
            state = (b == '\"') ? PS_Key : PS_Value;
            break;
        }
        case PS_CloseBrace:
        {
            pairvec_push(&pr.result, curPair);
            if (b == ',')
            {
                state = PS_OpenBrace;
            }
            else if (b == ']')
            {
                pr.success = true;
                goto done;
            }
            else
            {
                goto done;
            }
            break;
        }
        }
    }

done:
    strb_free(&curKey);
    strb_free(&curVal);
    return pr;
}