#pragma once

#include "io.hpp"

typedef struct {
    f64 x0, y0, x1, y1;
} HaversinePair;

f64 Haversine(HaversinePair p, f64 radius) {
    const Rad dLat = Deg(p.y1 - p.y0);
    const Rad dLon = Deg(p.x1 - p.x0);
    const Rad lat1 = Deg(p.y0);
    const Rad lat2 = Deg(p.y1);

    const f64 a = pow(sin(f32(dLat) * 0.5f), 2.0) + cos(f32(lat1)) * cos(f32(lat2)) * pow(sin(f32(dLon) * 0.5f), 2.0f);
    const f64 c = 2.0 * asin(sqrt(a));
    return radius * c;
}

f64 SumHaversines(Array<HaversinePair> pairs) {
    PROFILE_FUNCTION();
    if (pairs.len == 0) return 0.0;
    const f64 coef = 1.0 / (f64)pairs.len;
    f64       sum  = 0.0;
    for (size_t i = 0; i < pairs.len; ++i) {
        PROFILE_ADD_BANDWIDTH(sizeof(HaversinePair));
        sum += coef * Haversine(pairs[i], 6372.8);
    }
    return sum;
}

static f64 frand_unit() {
    return (f64)rand() / (f64)RAND_MAX;
}

static void GenerateHaversineJson(int count, cstr path) {
    PROFILE_FUNCTION();

    FILE* f = fopen(path, "wb");
    if (!f) {
        fprintf(stderr, "open %s failed\n", path);
        exit(1);
    }

    fputs("[\n", f);
    char buf[64];

    for (int i = 0; i < count; ++i) {
        PROFILE_ADD_BANDWIDTH(3 + 6 + 6 + 6 + 6 + 4);
        fputs("\t{\n", f);

        f64 v = frand_unit() * 160.0 - 80.0;
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
        if (i + 1 != count) fputc(',', f);
        fputc('\n', f);
    }
    fputs("]\n", f);
    PROFILE_ADD_BANDWIDTH(4);
    fclose(f);
}

typedef enum {
    PS_OpenBracket,
    PS_OpenBrace,
    PS_KeyStart,
    PS_Key,
    PS_Colon,
    PS_Value,
    PS_Comma,
    PS_CloseBrace
} HaversineParsingState;

typedef struct {
    cstr key;
    f64  value;
} JsonKeyValue;

Array<HaversinePair> ParseHaversineJson(Array<u8>& bytes, int expected_count = 1024) {
    PROFILE_FUNCTION();
    PROFILE_ADD_BANDWIDTH(bytes.cap);
    auto result = Array<HaversinePair>::New(expected_count);

    HaversineParsingState state  = PS_OpenBracket;
    Array<u8>             curKey = Array<u8>::New(128), curVal = Array<u8>::New(128);

    HaversinePair curPair = {};
    JsonKeyValue  curKv   = {};

    for (size_t i = 0; i < bytes.cap; ++i) {
        u8 b = bytes.data[i];

        if ((b == '\n' || b == '\t' || b == '\r' || b == ' ') && state != PS_Key) continue;

        switch (state) {
        case PS_OpenBracket: {
            if (b != '[')
                return result;
            else
                state = PS_OpenBrace;
            break;
        }
        case PS_OpenBrace: {
            if (b != '{')
                return result;
            else
                state = PS_KeyStart;
            break;
        }
        case PS_KeyStart: {
            if (b != '\"')
                return result;
            else
                state = PS_Key;
            break;
        }
        case PS_Key: {
            if (b == '\"') {
                state = PS_Colon;
            } else {
                curKey.Push((char)b);
            }
            break;
        }
        case PS_Colon: {
            if (b == ':') {
                curKey.Push('\0');
                curKv.key = (cstr) & (*curKey.data);
                state     = PS_Value;
            } else {
                return result;
            }
            break;
        }
        case PS_Value: {
            if (b == ',' || b == '}') {
                curKey.Push('\0');
                cstr val    = (cstr) & (*curVal.data);
                curKv.value = strtod(val, NULL);

                if (strcmp(curKv.key, "x0") == 0)
                    curPair.x0 = curKv.value;
                else if (strcmp(curKv.key, "y0") == 0)
                    curPair.y0 = curKv.value;
                else if (strcmp(curKv.key, "x1") == 0)
                    curPair.x1 = curKv.value;
                else if (strcmp(curKv.key, "y1") == 0)
                    curPair.y1 = curKv.value;

                curKey.len = 0;
                curVal.len = 0;

                state = (b == ',') ? PS_Comma : PS_CloseBrace;
            } else {
                curVal.Push(b);
                continue; // stay in Value
            }
            break;
        }
        case PS_Comma: {
            state = (b == '\"') ? PS_Key : PS_Value;
            break;
        }
        case PS_CloseBrace: {
            result.Push(curPair);
            if (b == ',') {
                state = PS_OpenBrace;
            } else if (b == ']') {
                return result;
            } else {
                return result;
            }
            break;
        }
        }
    }

    return result;
}