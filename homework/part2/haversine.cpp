#include "profiler.cpp"

struct File {
    FILE* data;

#ifdef DEBUG
    cstr path;
    u64  modTime;
#endif

#ifdef DEBUG
    File(cstr _path, cstr _mode) : data{fopen(_path, _mode)}, path{_path}, modTime{0} {}
#else
    File(cstr _path, cstr _mode) : data{fopen(_path, _mode)} {}
#endif

    ~File() { fclose(data); }

    operator FILE*() { return data; }
};

Array<u8> ReadEntireFile(const char* path) {
    PROFILE_FUNCTION();

    u64 rd = 0;

    File f(path, "rb");
    // FILE* f = fopen(path, "rb");
    if (!f) {
        fclose(f);
        return {};
    }

    if (fseek(f, 0, SEEK_END) != 0) return {};

    u64 cap = ftell(f);
    if (cap == 0) return {}; // == ?
    PROFILE_ADD_BANDWIDTH(cap);

    auto result = Array<u8>::New(cap);
    if (!result.data) return {};
    result.len = result.cap;

    rewind(f);
    rd = fread(result.data, 1, (u64)result.cap, f);
    fclose(f);
    f.data = 0;
    if (rd != (u64)result.cap) return {};

    return result;
}

typedef struct {
    double x0, y0, x1, y1;
} HaversinePair;

double Haversine(HaversinePair p, double radius) {
    const double dLat = Deg2Rad(p.y1 - p.y0);
    const double dLon = Deg2Rad(p.x1 - p.x0);
    const double lat1 = Deg2Rad(p.y0);
    const double lat2 = Deg2Rad(p.y1);

    const double a = pow(sin(dLat * 0.5), 2.0) + cos(lat1) * cos(lat2) * pow(sin(dLon * 0.5), 2.0);
    const double c = 2.0 * asin(sqrt(a));
    return radius * c;
}

double SumHaversines(Handle<HaversinePair> pairs, size_t n) {
    PROFILE_FUNCTION();
    if (n == 0) return 0.0;
    const double coef = 1.0 / (double)n;
    double       sum  = 0.0;
    for (size_t i = 0; i < n; ++i) {
        PROFILE_ADD_BANDWIDTH(sizeof(HaversinePair));
        sum += coef * Haversine(pairs[i], 6372.8);
    }
    return sum;
}

static double frand_unit(void) {
    return (double)rand() / (double)RAND_MAX;
}

static void GenerateHaversineJson(int count, const char* path) {
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
    const char* key;
    double      value;
} JsonKeyValue;

static Array<HaversinePair> ParseHaversineJson(Array<u8>& bytes, int expected_count = 1024) {
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
                const char* val = (cstr) & (*curVal.data);
                curKv.value     = strtod(val, NULL);

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