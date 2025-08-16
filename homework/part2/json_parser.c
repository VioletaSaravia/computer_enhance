#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "log.c"
#include "allocator.c"

typedef enum
{
    JSON_NULL,
    JSON_BOOL,
    JSON_STRING,
    JSON_NUMBER,
    JSON_OBJECT,
    JSON_ARRAY,
} JsonType;

typedef struct
{
    char *text;
    uint64_t len;
} String;

static Allocator Arena;
static Allocator TempArena;

typedef struct JsonValueNode JsonValueNode;

typedef struct JsonKeyValue JsonKeyValue;
typedef struct
{
    JsonKeyValue *kvs;
    uint64_t size;
} JsonObject;

typedef struct
{
    JsonType type;
    union
    {
        bool boolean;
        String string;
        double number;
        JsonObject object;
        JsonValueNode *array;
    };
} JsonValue;

struct JsonKeyValue
{
    String key;
    JsonValue value;
};

struct JsonValueNode
{
    JsonValue val;
    JsonValueNode *next;
};

typedef struct
{
    String input;
    uint64_t at, to;
    JsonValue output;
} JsonParser;

char ParserIter(JsonParser *p)
{
    char result = '\0';

    if (p->at < p->input.len)
    {
        result = p->input.text[p->at];
        p->at++;
    }

    return result;
}

uint64_t SimpleHash(String str)
{
    uint64_t hash = 0;
    for (uint64_t i = 0; i < str.len; i++)
    {
        hash += str.text[i];
    }
    return hash;
}

// Djb2 hash
uint64_t Hash(String str)
{
    uint64_t hash = 5381;
    for (uint64_t i = 0; i < str.len; i++)
    {
        hash = ((hash << 5) + hash) ^ str.text[i];
    }
    return hash;
}

bool CharCanBeIgnored(char c)
{
    return c == ' ' || c == '\n' || c == '\t' || c == '\r';
}

void ParseJsonNumber(JsonParser *p, double *val)
{
    char *end = 0;
    *val = strtod(p->input.text + p->at, &end);

    p->at = end != 0 ? end - p->input.text - 1 : p->input.len - 1;
}

void ParseJsonBool(JsonParser *p, bool *val)
{
    if (p->input.text[p->at] == 'f' && p->input.len >= p->at + 4)
    {
        if (memcmp((void *)&p->input.text[p->at], (void *)"false", 5) != 0)
        {
            LOG_FATAL("Expected FALSE");
        }

        *val = false;
        p->at += 4;
        return;
    }

    if (p->input.text[p->at] == 't' && p->input.len >= p->at + 3)
    {
        if (memcmp((void *)&p->input.text[p->at], (void *)"true", 4) != 0)
        {
            LOG_FATAL("Expected TRUE");
        }

        *val = false;
        p->at += 3;
        return;
    }

    LOG_FATAL("Reached EOF while parsing BOOL");
}

void ParseJsonNull(JsonParser *p)
{
    if (memcmp((void *)&p->input.text[p->at], "null", 4) == 0)
    {
        p->at += 3;
    }
    else
    {
        LOG_FATAL("Expected NULL");
    }
}

void ParseJsonValue(JsonParser *p, JsonValue *val);

typedef enum
{
    ARRAY_BEGIN,
    ARRAY_VALUE,
    ARRAY_COMMA_OR_END
} ArrayPosition;

void ParseJsonArray(JsonParser *p, JsonValueNode **val)
{
    JsonValueNode *newNode = 0;
    ArrayPosition next = ARRAY_BEGIN;

    for (; p->at < p->input.len - 1; p->at++)
    {
        char ch = p->input.text[p->at];

        switch (next)
        {
        case ARRAY_BEGIN:
            if (CharCanBeIgnored(ch))
            {
                continue;
            }
            else if (ch == '[')
            {
                if (p->input.text[p->at + 1] == ']') // Empty array
                {
                    p->at++;
                    return;
                }

                next = ARRAY_VALUE;
            }
            break;

        case ARRAY_VALUE:
            newNode = Alloc(&Arena, sizeof(JsonValueNode));
            ParseJsonValue(p, &newNode->val);

            newNode->next = *val;
            *val = newNode;

            next = ARRAY_COMMA_OR_END;
            break;

        case ARRAY_COMMA_OR_END:
            if (CharCanBeIgnored(ch))
            {
                continue;
            }
            else if (ch == ',')
            {
                next = ARRAY_VALUE;
                continue;
            }
            else if (ch == ']')
            {
                return;
            }

        default:
            LOG_FATAL("Invalid character while parsing array");
        }
    }

    LOG_FATAL("Reached EOF while parsing array");
}

void StrCpy(String to, char *source)
{
    for (uint64_t i = 0; i < to.len; i++)
    {
        if (source[i] == '\0')
        {
            LOG_WARNING("Source char* is shorter than target string's length");
            return;
        }
        to.text[i] = source[i];
    }
}

typedef enum
{
    STRING_BEGIN_QUOTE,
    STRING_TEXT,
    STRING_END_QUOTE,
} StringPosition;

void ParseJsonString(JsonParser *p, String *val)
{
    StringPosition next = STRING_BEGIN_QUOTE;
    for (; p->at + p->to < p->input.len; p->at++)
    {
        char ch = p->input.text[p->at + p->to];

        switch (next)
        {
        case STRING_BEGIN_QUOTE:
            if (CharCanBeIgnored(ch))
            {
                continue;
            }
            else if (ch == '\"')
            {
                next = STRING_TEXT;
                continue;
            }
            else
            {
                LOG_FATAL("Invalid char while parsing string");
            }

        case STRING_TEXT:
            if (ch != '\"')
            {
                p->to++;
                p->at--;
                continue;
            }
            else
            {
                next = STRING_END_QUOTE;
                p->at--;
                continue;
            }

        case STRING_END_QUOTE:
            *val = (String){.text = Alloc(&Arena, p->to), .len = p->to};
            StrCpy(*val, &p->input.text[p->at]);

            p->at += p->to;
            p->to = 0;
            return;

        default:
            break;
        }
    }

    LOG_FATAL("Reached EOF while parsing string");
}

typedef enum
{
    PARSING_BEGIN,
    PARSING_KEY,
    PARSING_COLON,
    PARSING_VALUE,
    PARSING_COMMA_OR_END,
} ObjectPosition;

void ParseJsonObject(JsonParser *p, JsonObject *val)
{
    *val = (JsonObject){
        .size = 256,
        .kvs = Alloc(&Arena, sizeof(JsonKeyValue) * 12)};

    ObjectPosition next = PARSING_BEGIN;

    String curKey = {0};
    uint64_t curHash = 0;

    for (; p->at < p->input.len; p->at++)
    {
        char ch = p->input.text[p->at];

        switch (next)
        {
        case PARSING_BEGIN:
            if (CharCanBeIgnored(ch))
            {
                continue;
            }
            else if (ch == '{')
            {
                if (p->input.text[p->at + 1] == '}')
                {
                    p->at++;
                    return;
                }
                next = PARSING_KEY;
                continue;
            }
            UNREACHABLE;

        case PARSING_KEY:
            ParseJsonString(p, &curKey);
            curHash = Hash(curKey) % val->size;
            if (val->kvs->key.text != 0)
            {
                LOG_WARNING("Hash collision");
            }

            val->kvs[curHash].key = curKey;

            next = PARSING_COLON;
            break;

        case PARSING_VALUE:
            ParseJsonValue(p, &val->kvs[curHash].value);

            next = PARSING_COMMA_OR_END;
            break;

        case PARSING_COMMA_OR_END:
            if (CharCanBeIgnored(ch))
            {
                continue;
            }
            else if (ch == ',')
            {
                next = PARSING_KEY;
                continue;
            }
            else if (ch == '}')
            {
                return;
            }
            else
            {
                LOG_FATAL("Invalid character, expected COMMA or CLOSE_BRACKET");
            }

        case PARSING_COLON:
            if (CharCanBeIgnored(ch))
            {
                continue;
            }
            else if (ch == ':')
            {
                next = PARSING_VALUE;
                break;
            }
            else
            {
                LOG_FATAL("Invalid character, expected COLON");
            }

        default:
            break;
        }
    }

    LOG_FATAL("Reached EOF while parsing OBJECT");
}

void ParseJsonValue(JsonParser *p, JsonValue *val)
{
    while (true)
    {
        char ch = p->input.text[p->at];
        if (CharCanBeIgnored(ch))
        {
            p->at++;
            continue;
        }
        else if (ch == '{')
        {
            val->type = JSON_OBJECT;
            ParseJsonObject(p, &val->object);
            return;
        }
        else if ((ch >= '0' && ch <= '9') || ch == '-' || ch == '.')
        {
            val->type = JSON_NUMBER;
            ParseJsonNumber(p, &val->number);
            return;
        }
        else if (ch == 't' || ch == 'f')
        {
            val->type = JSON_BOOL;
            ParseJsonBool(p, &val->boolean);
            return;
        }
        else if (ch == 'n')
        {
            val->type = JSON_NULL;
            ParseJsonNull(p);
            return;
        }
        else if (ch == '[')
        {
            val->type = JSON_ARRAY;
            ParseJsonArray(p, &val->array);
            return;
        }
        else if (ch == '\"')
        {
            val->type = JSON_STRING;
            ParseJsonString(p, &val->string);
            return;
        }
        else
        {
            LOG_FATAL("Invalid character");
        }
    }
}

void JsonPrint(JsonValue val)
{
    switch (val.type)
    {
    case JSON_NULL:
        printf("null");
        break;
    case JSON_BOOL:
        printf(val.boolean ? "true" : "false");
        break;
    case JSON_STRING:
        printf("\"%.*s\"", (int)(val.string.len), val.string.text);
        break;
    case JSON_NUMBER:
        printf("%.2f", val.number);
        break;
    case JSON_OBJECT:
    {
        printf("{");

        uint64_t printed = 0;
        bool first = true;
        for (uint64_t i = 0; i < val.object.size; i++)
        {
            JsonKeyValue next = val.object.kvs[i];
            if (next.key.text == 0)
                continue;

            printed++;
            if (first)
                first = false;
            else
                printf(",");

            JsonPrint((JsonValue){.type = JSON_STRING, .string = next.key});
            printf(":");
            JsonPrint(next.value);
        }
        printf(printed != 0 ? "}" : "}");
    }
    break;
    case JSON_ARRAY:
    {
        printf("[");

        bool first = true;
        for (JsonValueNode *next = val.array; next; next = next->next)
        {
            if (first)
                first = false;
            else
                printf(",");
            JsonPrint(next->val);
        }

        printf("]");
    }
    break;
    default:
        LOG_WARNING("Invalid value type");
        return;
    }
}

String LoadFile(const char *path, Allocator allocator)
{
    FILE *f = fopen(path, "r");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *string = Alloc(&allocator, fsize + 1);

    fread(string, fsize, 1, f);
    fclose(f);

    string[fsize] = 0;

    return (String){
        .len = fsize + 1,
        .text = string,
    };
}

JsonValue LoadJson(String json)
{
    JsonParser parser = {.input = json};
    ParseJsonValue(&parser, &parser.output);
    return parser.output;
}

#define KB(val) val * 1024
#define MB(val) KB(val) * 1024
#define GB(val) MB(val) * 1024

int main(int argc, char const *argv[])
{
    PROFILE("allocating arenas",
            Arena = NewArena(GB(1));
            TempArena = NewArena(GB(1)););

    String file = {0};
    PROFILE("loading file",
            const char *path = argc > 1 ? argv[1] : "input_uniform.json";
            file = LoadFile(path, TempArena););

    JsonValue test = {0};
    PROFILE("parsing json",
            test = LoadJson(file););

    if (argc > 2 && strcmp(argv[2], "print") == 0)
    {
        PROFILE("printing parsed json",
                JsonPrint(test);
                printf("\n"));
    }

    return 0;
}