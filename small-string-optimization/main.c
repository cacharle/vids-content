#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

struct StringLargeStorage {
    char *data;
    size_t capacity;
};

// TODO: need something else than length since we can have a large capacity with a 0 length
// clang's implementation looks quite nice:
//
//  union {
//     struct { size_t is_long:1, cap:63; size_t size; char *data; } l;
//     struct { unsigned char is_long:1, size:7; char data[23]; }    s;
// };

#define STRING_SMALL_STORAGE_SIZE sizeof(struct StringLargeStorage)

struct String {
    union {
        struct StringLargeStorage large;
        char small[STRING_SMALL_STORAGE_SIZE];
    };
    size_t length;
};

typedef struct String String;

String string_new() {
    String s;
    s.length = 0;
    s.small[0] = '\0';
    return s;
}

static bool is_small(String *s) {
    return s->length < STRING_SMALL_STORAGE_SIZE;
}

char *string_data(String *s) {
    if (is_small(s))
        return s->small;
    else
        return s->large.data;
}

static void grow_for(String *s, size_t added_capacity) {
    if (!is_small(s)) {
        if (s->length + added_capacity >= STRING_SMALL_STORAGE_SIZE)
        {
            char tmp[STRING_SMALL_STORAGE_SIZE];
            memcpy(tmp, s->small, STRING_SMALL_STORAGE_SIZE);
            s->large.capacity = s->length + STRING_SMALL_STORAGE_SIZE;
            s->large.data = malloc(s->large.capacity * sizeof(char));
            memcpy(s->large.data, tmp, STRING_SMALL_STORAGE_SIZE);
        }
    } else {
        s->large.capacity += added_capacity;
        s->large.data = realloc(s->large.data, s->large.capacity * sizeof(char));
    }
}

void string_push_char(String *s, char c) {
    grow_for(s, 1);
    if (is_small(s)) {
        s->small[s->length] = c;
        s->length++;
        s->small[s->length] = '\0';
    } else {
        s->large.data[s->length] = c;
        s->length++;
        s->large.data[s->length] = '\0';
    }
}

int main() {
    String s = string_new();
    return 0;
}
