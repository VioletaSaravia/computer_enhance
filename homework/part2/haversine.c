#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

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
    uint64_t len;
    JsonValue *data;
} JsonArray;

typedef struct JsonObject JsonObject;
struct JsonObject
{
    JsonObject *data;
    uint64_t len, size;

    JsonObject *(*Get)(String str);
};

struct JsonValue
{
    JsonType type;
    union
    {
        uint8_t null;
        uint8_t bool;
        String string;
        double_t number;
        JsonObject object;
        JsonArray array;
    };
};

typedef struct
{
    FILE *file;
    uint64_t at, to;
    JsonValue output;
} JsonParser;

uint64_t Hash(String str)
{
    uint64_t hash = 0;
    for (uint64_t i = 0; i < str.len; i++)
    {
        hash += str.text[i];
    }
    return hash;
}

typedef enum
{
    PARSING_NONE,
    PARSING_STRING,
    PARSING_VALUE,
} JsonParsingState;

void ParseJsonObject(String str, JsonValue *val)
{
    val->type = JSON_OBJECT;
    val->object = (JsonObject){
        .data = malloc(1),
        .Get = Hash,
        .len = 0,
        .size = 999,
    };

    JsonParsingState parsing = PARSING_NONE;

    char ch = EOF;
loop:
    for (uint64_t i = 0; i < str.len; i++)
    {
        switch (parsing)
        {
        case PARSING_NONE:
            if (ch == ' ')
            {
                goto loop;
            }
            else if (ch == '\"')
            {
                parsing = PARSING_STRING;
            }
            else if (ch == ':')
            {
                abort();
            }
            else if (ch == ',')
            {
                abort();
            }
            break;

        case PARSING_STRING:
            break;

        case PARSING_VALUE:
            break;

        default:
            break;
        }
    }
}

void ParseJsonNumber(String str, JsonValue *val)
{
    int8_t parsing = 0;

    char ch = EOF;
    for (uint64_t i = 0; i < str.len; i++)
    {

        if (ch == ' ' || ch == '\t' || ch == '\n')
        {
            continue;
        }
        else if (ch >= '0' && ch <= '9')
        {
            parsing = 1;
        }
        else
        {
            abort();
        }
    }
}
void ParseJsonBool(String str, JsonValue *val)
{
}
void ParseJsonNull(String str, JsonValue *val) {}
void ParseJsonArray(String str, JsonValue *val) {}
void ParseJsonString(String str, JsonValue *val) {}

void ParseJsonValue(String str, JsonValue *val)
{
    char ch = EOF;
    for (uint64_t i = 0; i < str.len; i++)
    {
        printf("Parsing: %c\n", ch);
        if (ch == '{')
        {
            ParseJsonObject(str, val);
        }
        else if (ch >= '0' && ch <= '9')
        {
            ParseJsonNumber(str, val);
        }
        else if (ch == 't' || ch == 'f')
        {
            ParseJsonBool(str, val);
        }
        else if (ch == 'n')
        {
            ParseJsonNull(str, val);
        }
        else if (ch == '[')
        {
            ParseJsonArray(str, val);
        }
        else if (ch == '\"')
        {
            ParseJsonString(str, val);
        }
        else if (ch == ' ' || ch == '\n' || ch == '\t')
        {
            continue;
        }
        else
        {
            printf("Invalid character: %c\n", ch);
            abort();
        }
    }
}

int32_t main(int argc, char const *argv[])
{
    // TODO: Read fixed buf, not whole file
    FILE *f = fopen("homework/part2/input_uniform.json", "r");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET); /* same as rewind(f); */

    char *string = malloc(fsize + 1);
    fread(string, fsize, 1, f);
    fclose(f);

    string[fsize] = 0;

    String str = (String){
        .len = fsize + 1,
        .text = string,
    };

    JsonParser parser = {0};

    if (!parser.file)
    {
        printf("Couldn't open file\n");
        abort();
    }

    ParseJsonValue(str, &parser.output);

    free(string);
    return 0;
}
