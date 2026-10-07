#include <stdio.h>
#include <stdlib.h>

// with flexible

struct Vec {
    size_t capacity;
    size_t length;
    int content[];
};
typedef struct Vec Vec;

Vec* vec_new(size_t capacity) {
    Vec *v = malloc(sizeof(Vec) + capacity * sizeof(int));
    v->capacity = capacity;
    v->length = 0;
    return v;
}
void vec_destroy(Vec *v) { free(v); }

void vec_push(Vec **super_v, int x) {
    Vec *v = *super_v;
    if (v->length == v->capacity) {
        v->capacity *= 2;
        v = realloc(v, sizeof(Vec) + v->capacity * sizeof(int));
    }
    v->content[v->length] = x;
    v->length++;
    *super_v = v;
}

// without flexible
// struct Vec {
//     size_t capacity;
//     size_t length;
//     int *content;
// };
// typedef struct Vec Vec;
//
// Vec* vec_new(size_t capacity) {
//     Vec *v = malloc(sizeof(Vec));
//     v->content = malloc(capacity * sizeof(int));
//     v->capacity = capacity;
//     v->length = 0;
//     return v;
// }
// void vec_destroy(Vec *v) { free(v->content); free(v); }
//
// void vec_push(Vec **super_v, int x) {
//     Vec *v = *super_v;
//     if (v->length == v->capacity) {
//         v->capacity *= 2;
//         v->content = realloc(v->content, v->capacity * sizeof(int));
//     }
//     v->content[v->length] = x;
//     v->length++;
//     *super_v = v;
// }

int main()
{
    Vec *v = vec_new(4);
    for (int i = 0; i < 10; i++)
        vec_push(&v, i * i);

    for (size_t i = 0; i < v->length; i++)
        printf("%d, ", v->content[i]);

    return 0;
}
