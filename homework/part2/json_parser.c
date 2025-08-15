#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

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

typedef struct JsonValue JsonValue;
typedef struct
{
    uint64_t len, size;
    JsonValue *data;
} JsonArray;

typedef struct JsonObject JsonObject;
struct JsonObject
{
    JsonValue *data;
    String *keys;
    uint64_t len, size;
};

struct JsonValue
{
    JsonType type;
    union
    {
        bool boolean;
        String string;
        double_t number;
        JsonObject object;
        JsonArray array;
    };
};

typedef struct
{
    String input;
    uint64_t at, to;
    JsonValue output;
} JsonParser;

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
            abort();

        *val = false;
        p->at += 4;
        return;
    }

    if (p->input.text[p->at] == 't' && p->input.len >= p->at + 3)
    {
        if (memcmp((void *)&p->input.text[p->at], (void *)"true", 4) != 0)
            abort();

        *val = false;
        p->at += 3;
        return;
    }

    abort();
}

void ParseJsonNull(JsonParser *p)
{
    if (memcmp((void *)&p->input.text[p->at], "null", 4) == 0)
    {
        p->at += 4;
    }
    else
    {
        abort();
    }
}

void ParseJsonValue(JsonParser *p, JsonValue *val);

typedef enum
{
    ARRAY_BEGIN,
    ARRAY_VALUE,
    ARRAY_COMMA_OR_END
} ArrayPosition;

void ParseJsonArray(JsonParser *p, JsonArray *val)
{
    ArrayPosition next = ARRAY_BEGIN;
    *val = (JsonArray){
        .data = malloc(sizeof(JsonValue) * 1),
        .len = 0,
        .size = 1,
    };

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
            if (val->len == val->size)
            {
                val->size *= 2;
                val->data = realloc(val->data, sizeof(JsonValue) * val->size);
            }
            ParseJsonValue(p, &val->data[val->len]);
            val->len++;

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
            else
            {
                abort();
            }

        default:
            abort();
        }
    }

    printf("[ERROR] Reached EOF while parsing array\n");
    abort();
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
                printf("Invalid char %c while parsing string", ch);
                abort();
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
            *val = (String){.text = &p->input.text[p->at], .len = p->to};
            p->at += p->to;
            p->to = 0;
            return;

        default:
            break;
        }
    }

    printf("[ERROR] Reached EOF while parsing string");
    abort();
}

typedef enum
{
    PARSING_BEGIN,
    PARSING_KEY,
    PARSING_COLON,
    PARSING_VALUE,
    PARSING_COMMA_OR_END,
} ObjectPosition;

JsonValue *JsonObjGet(JsonObject o, String key)
{
    uint64_t hash = Hash(key) % o.size;
    // TODO
    for (uint64_t i = 0; i < o.size - 1 && strcmp(key.text, o.keys[(hash + i) % o.size].text) != 0, i++;)
    {
    }
    return &o.data[hash];
}

void ParseJsonObject(JsonParser *p, JsonObject *val)
{
    *val = (JsonObject){
        .keys = malloc(sizeof(String) * 256),
        .data = malloc(sizeof(JsonValue) * 256),
        .len = 0,
        .size = 256,
    };

    ObjectPosition next = PARSING_BEGIN;

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
            abort(); // unreachable

        case PARSING_KEY:
            if (val->len == val->size)
            {
                val->size *= 2;
                val->keys = realloc(val->keys, val->size * sizeof(String));
                val->data = realloc(val->data, val->size * sizeof(JsonValue));
            }
            ParseJsonString(p, &val->keys[val->len]);
            next = PARSING_COLON;
            break;

        case PARSING_VALUE:
            ParseJsonValue(p, &val->data[Hash(val->keys[val->len]) % val->size]);
            val->len++;
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
                abort();
            }

        default:
            break;
        }
    }

    printf("EOF\n");
    abort();
}

void ParseJsonValue(JsonParser *p, JsonValue *val)
{
    while (true)
    {
        char ch = p->input.text[p->at];
        if (ch == '{')
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
        else if (CharCanBeIgnored(ch))
        {
            p->at++;
            continue;
        }
        else
        {
            printf("Invalid character: %c\n", ch);
            abort();
        }
    }
}

void JsonPrintBool(bool val)
{
    printf(val ? "true" : "false");
}

void JsonPrintString(String val)
{
    printf("\"%.*s\"", (int)(val.len), val.text);
}

void JsonPrintNumber(double val)
{
    printf("%.2f", val);
}

void JsonPrint(JsonValue val);

void JsonPrintObject(JsonObject val)
{
    printf("{");

    for (uint64_t i = 0; i < val.len; i++)
    {
        printf("\n\t");
        JsonPrint((JsonValue){.type = JSON_STRING, .string = val.keys[i]});
        printf(": ");
        JsonPrint(val.data[Hash(val.keys[i]) % val.size]);
        if (i + 1 < val.len)
        {
            printf(",");
        }
    }
    printf(val.len != 0 ? "\n}" : "}");
}

void JsonPrintArray(JsonArray val)
{
    printf("[");
    for (uint64_t i = 0; i < val.len; i++)
    {
        printf("\n\t");
        JsonPrint(val.data[i]);
        if (i + 1 < val.len)
            printf(",");
    }
    printf("\n]");
}

void JsonPrint(JsonValue val)
{
    switch (val.type)
    {
    case JSON_NULL:
        printf("null");
        break;
    case JSON_BOOL:
        JsonPrintBool(val.boolean);
        break;
    case JSON_STRING:
        JsonPrintString(val.string);
        break;
    case JSON_NUMBER:
        JsonPrintNumber(val.number);
        break;
    case JSON_OBJECT:
        JsonPrintObject(val.object);
        break;
    case JSON_ARRAY:
        JsonPrintArray(val.array);
        break;
    default:
        printf("[Warning] Invalid print value type: %i", val.type);
        return;
    }
}

JsonValue LoadJson(const char *path)
{
    const char *filePath = path ? path : "input_uniform.json";
    FILE *f = fopen(filePath, "r");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *string = (char *)malloc(fsize + 1);

    fread(string, fsize, 1, f);
    fclose(f);

    string[fsize] = 0;

    JsonParser parser = {.input = (String){
                             .len = fsize + 1,
                             .text = string,
                         }};

    printf("[INFO] Started parsing %s\n", filePath);
    ParseJsonValue(&parser, &parser.output);
    // JsonPrint(parser.output);
    // printf("\n");
    printf("[INFO] Finished parsing %s\n", filePath);

    return parser.output;
}

int32_t main(int argc, char const *argv[])
{
    JsonValue test = LoadJson(NULL);
    return 0;
}
