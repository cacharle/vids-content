#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// clang's implementation looks quite nice:
//
//  union {
//     struct { size_t is_long:1, cap:63; size_t size; char *data; } l;
//     struct { unsigned char is_long:1, size:7; char data[23]; }    s;
// };

#define STRING_SMALL_STORAGE_SIZE sizeof(char*)

struct String {
    union {
        char *large;
        char small[STRING_SMALL_STORAGE_SIZE];
    };
    size_t length;
    size_t capacity;
};

typedef struct String String;

String string_new() {
    String s;
    s.length = 0;
    s.capacity = STRING_SMALL_STORAGE_SIZE - 1;
    s.small[0] = '\0';
    return s;
}

static bool is_small(String *s) {
    return s->capacity < STRING_SMALL_STORAGE_SIZE;
}

char *string_data(String *s) {
    if (is_small(s))
        return s->small;
    else
        return s->large;
}

static void grow_for(String *s, size_t added_capacity) {
    if (is_small(s)) {
        if (s->length + added_capacity >= STRING_SMALL_STORAGE_SIZE)
        {
            char tmp[STRING_SMALL_STORAGE_SIZE];
            memcpy(tmp, s->small, STRING_SMALL_STORAGE_SIZE);
            s->capacity = s->length + STRING_SMALL_STORAGE_SIZE;
            s->large = malloc(s->capacity * sizeof(char));
            memcpy(s->large, tmp, STRING_SMALL_STORAGE_SIZE);
        }
    } else {
        s->capacity += added_capacity;
        s->large = realloc(s->large, s->capacity * sizeof(char));
    }
}

void string_push_char(String *s, char c) {
    grow_for(s, 1);
    if (is_small(s)) {
        s->small[s->length] = c;
        s->length++;
        s->small[s->length] = '\0';
    } else {
        s->large[s->length] = c;
        s->length++;
        s->large[s->length] = '\0';
    }
}

String string_copy(String *s)
{
    if (is_small(s))
        return *s;
    else {
        String new_string = string_new();
        new_string.capacity = s->capacity;
        new_string.length = s->capacity;
        new_string.large = malloc(new_string.capacity);
        memcpy(new_string.large, s->large, new_string.capacity);
        return new_string;
    }
}

int main() {
    String s = string_new();
    string_push_char(&s, 'a');
    string_push_char(&s, 'b');
    string_push_char(&s, 'c');
    string_push_char(&s, 'c');
    string_push_char(&s, 'c');
    string_push_char(&s, 'c');
    string_push_char(&s, 'c');
    string_push_char(&s, 'c');
    string_push_char(&s, 'c');
    string_push_char(&s, 'c');
    string_push_char(&s, 'c');
    string_push_char(&s, 'c');
    string_push_char(&s, 'c');
    string_push_char(&s, 'c');
    string_push_char(&s, 'c');
    string_push_char(&s, 'c');
    string_push_char(&s, 'c');
    string_push_char(&s, 'c');
    string_push_char(&s, 'c');
    string_push_char(&s, 'c');
    string_push_char(&s, 'c');
    String x = string_copy(&s);
    printf("%s\n", string_data(&s));
    printf("%s\n", string_data(&x));
    return 0;
}
