#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#undef GROW_FACTOR
#undef SHRINK_FACTOR

#define GROW_FACTOR 2.f
#define SHRINK_FACTOR 0.5f
#define MIN_SET 8
#define LOW_LOAD_FACTOR 0.1f
#define HIGH_LOAD_FACTOR 0.75f
#define INVALID -1

#ifndef HELPER_MACROS
    #define HELPER_MACROS
#endif

// credit Gustav Louw for this idea
#ifdef HELPER_MACROS
    #define CAT(a, b) a##b
    #define PASTE(a, b) CAT(a, b)
    #define JOIN(name, suffix) PASTE(name, PASTE(_, suffix))

    #define M_BUCKET(type) JOIN(SetBucket, type)
    #define M_SET(type) JOIN(Set, type)
    #define M_ITER(type) JOIN(SetIterator, type)
#endif

#define set_constructor(type)                                                           \
    set_constructor_custom(type, JOIN(compare_general, type), JOIN(hash_general, type)) \

#define set_constructor_custom(type, cmp, hsh)                                            \
{                                                                                         \
    ._array = malloc(sizeof(M_BUCKET(type)) * 0),                                         \
    ._capacity = 0, ._elements = 0, ._type_size = sizeof(M_BUCKET(type)),                 \
    ._cmp = cmp, ._hash = hsh,                                                            \
    .begin = JOIN(begin, type), .next = JOIN(next, type), .end = JOIN(end, type),         \
    .insert = JOIN(insert, type), .erase = JOIN(erase, type), .clear = JOIN(clear, type), \
    .contains = JOIN(contains, type), .empty = JOIN(empty, type),                         \
    .size = JOIN(size, type), .capacity = JOIN(capacity, type)                            \
}

// CONSIDER: cpp-reference specifies red-black trees as typical set implementation
//  I hate trees, this implementation is very similar to a hash table but without key-value pairs

// slightly modified version of djb2 algorithm to allow handling of null terminators (e.g. hash an int 0)
// there are other ways to accomplish this (e.g. snprintf) but this method is simple and general purpose
unsigned long djb2(const unsigned char *str, size_t len)
{
    unsigned long hash = 5381;

    for (int i = 0; i < len; i++)
        hash = ((hash << 5) + hash) + str[i];

    return hash;
}
unsigned int get_index(unsigned long hash, size_t capacity)
{
    return hash % capacity;
}

// the _cmp and _hash function pointers are plug-n-play
// for structs or other complex comparisons/hashes
// they can be substituted with custom functions
// plug-n-play functions <--> can be swapped out for custom functions if needed

#define SET(type)                                                                            \
                                                                                             \
bool JOIN(compare_general, type)(type a, type b)                                             \
{                                                                                            \
    return a == b;                                                                           \
}                                                                                            \
                                                                                             \
bool JOIN(compare_string, type)(const char* a, const char* b)                                \
{                                                                                            \
    return !strcmp(a, b);                                                                    \
}                                                                                            \
                                                                                             \
unsigned long JOIN(hash_general, type)(type item)                                            \
{                                                                                            \
    size_t len = sizeof(item);                                                               \
    unsigned char data[len];                                                                 \
    memcpy(data, &item, len);                                                                \
                                                                                             \
    return djb2(data, len);                                                                  \
}                                                                                            \
                                                                                             \
unsigned long JOIN(hash_string, type)(const char* item)                                      \
{                                                                                            \
    size_t len = strlen(item);                                                               \
    unsigned char data[len];                                                                 \
    memcpy(data, &item, len);                                                                \
                                                                                             \
    return djb2(data, len);                                                                  \
}                                                                                            \
                                                                                             \
typedef struct M_BUCKET(type)                                                                \
{                                                                                            \
    unsigned long hash;                                                                      \
    type value;                                                                              \
    bool tombstone;                                                                          \
} M_BUCKET(type), M_ITER(type);                                                              \
                                                                                             \
typedef struct M_SET(type)                                                                   \
{                                                                                            \
    M_BUCKET(type)* _array;                                                                  \
    size_t _capacity;                                                                        \
    size_t _elements;                                                                        \
    size_t _type_size;                                                                       \
                                                                                             \
    bool (*_cmp)(type, type);                                                                \
    unsigned long (*_hash)(type);                                                            \
                                                                                             \
    M_ITER(type) *(*begin)(struct Set_int*);                                                 \
    M_ITER(type) *(*next)(struct Set_int*, M_ITER(type)*);                                   \
    M_ITER(type) *(*end)(struct Set_int*);                                                   \
                                                                                             \
    void (*insert)(struct M_SET(type)*, type);                                               \
    void (*erase)(struct M_SET(type)*, type);                                                \
    bool (*contains)(struct M_SET(type)*, type);                                             \
    bool (*empty)(struct M_SET(type)*);                                                      \
    size_t (*size)(struct M_SET(type)*);                                                     \
    size_t (*capacity)(struct M_SET(type)*);                                                 \
    void (*clear)(struct M_SET(type)*);                                                      \
} M_SET(type);                                                                               \
                                                                                             \
M_ITER(type) *begin_int(M_SET(type)* set)                                                    \
{                                                                                            \
    M_ITER(type) *iter = set->_array;                                                        \
    while (                                                                                  \
        iter < (set->_array + set->_capacity) && (iter->hash == INVALID || iter->tombstone)  \
    ) ++iter;                                                                                \
    return iter;                                                                             \
}                                                                                            \
                                                                                             \
M_ITER(type) *next_int(M_SET(type)* set, M_ITER(type)* iter)                                 \
{                                                                                            \
    do {                                                                                     \
        ++iter;                                                                              \
    } while (                                                                                \
        iter < (set->_array + set->_capacity) && (iter->hash == INVALID || iter->tombstone)  \
    );                                                                                       \
    return iter;                                                                             \
}                                                                                            \
                                                                                             \
M_ITER(type) *end_int(Set_int* set)                                                          \
{                                                                                            \
    return set->_array + set->_capacity;                                                     \
}                                                                                            \
                                                                                             \
unsigned int                                                                                 \
JOIN(h_lprobe, type)(M_SET(type) *set, int value, unsigned int index, bool skip_tombstones)  \
{                                                                                            \
    assert(set && set->_array);                                                              \
                                                                                             \
    unsigned int found = -1;                                                                 \
    unsigned int tombstone = -1;                                                             \
    unsigned int count = 1;                                                                  \
                                                                                             \
    while (found == -1)                                                                      \
    {                                                                                        \
        M_BUCKET(type) bucket = set->_array[index];                                          \
                                                                                             \
        if (skip_tombstones && tombstone == INVALID &&                                       \
            bucket.hash != INVALID && bucket.tombstone)                                      \
            tombstone = index;                                                               \
        else if (set->_cmp(bucket.value, value) || (!skip_tombstones &&                      \
            (bucket.hash == INVALID || bucket.tombstone)))                                   \
            found = index;                                                                   \
        else if (skip_tombstones && bucket.hash == INVALID)                                  \
            break;                                                                           \
        else                                                                                 \
            index = (index + 1) % set->_capacity;                                            \
                                                                                             \
        if (++count > set->_capacity)                                                        \
            break;                                                                           \
    }                                                                                        \
                                                                                             \
    if (tombstone != -1 && found != -1)                                                      \
    {                                                                                        \
        set->_array[tombstone].value = set->_array[found].value;                             \
        set->_array[tombstone].tombstone = false;                                            \
        set->_array[found].value = INVALID;                                                  \
        set->_array[found].tombstone = true;                                                 \
                                                                                             \
        found = tombstone;                                                                   \
    }                                                                                        \
                                                                                             \
    return found;                                                                            \
}                                                                                            \
                                                                                             \
void JOIN(h_resize, type)(M_SET(type) *set, float factor)                                    \
{                                                                                            \
    assert(set->_array);                                                                     \
                                                                                             \
    size_t size = set->_type_size * set->_capacity;                                          \
    size_t capacity = set->_capacity;                                                        \
                                                                                             \
    M_BUCKET(type)* tmp = malloc(size);                                                      \
    memcpy(tmp, set->_array, size);                                                          \
    free(set->_array);                                                                       \
                                                                                             \
    set->_capacity = (size_t)((float)set->_capacity * factor);                               \
    set->_array = malloc(set->_type_size * set->_capacity);                                  \
    assert(set->_array);                                                                     \
    memset(set->_array, INVALID, (size_t)((float)size * factor));                            \
                                                                                             \
    M_BUCKET(type) bucket;                                                                   \
    for (int i = 0; i < capacity; i++)                                                       \
    {                                                                                        \
        bucket = tmp[i];                                                                     \
                                                                                             \
        if (bucket.hash == INVALID || bucket.tombstone)                                      \
            continue;                                                                        \
                                                                                             \
        unsigned int re_index = get_index(bucket.hash, set->_capacity);                      \
        re_index = JOIN(h_lprobe, type)(set, bucket.value, re_index, false);                 \
        set->_array[re_index] = bucket;                                                      \
    }                                                                                        \
                                                                                             \
    free(tmp);                                                                               \
}                                                                                            \
                                                                                             \
void JOIN(insert, type)(M_SET(type)* set, type value)                                        \
{                                                                                            \
    if (set->_elements == 0)                                                                 \
    {                                                                                        \
        set->_capacity = MIN_SET;                                                            \
        free(set->_array);                                                                   \
        set->_array = malloc(set->_type_size * set->_capacity);                              \
        memset(set->_array, INVALID, set->_type_size * set->_capacity);                      \
    }                                                                                        \
    else                                                                                     \
    {                                                                                        \
        float load_factor = ((float)set->_elements / (float)set->_capacity);                 \
        if (load_factor >= HIGH_LOAD_FACTOR)                                                 \
            JOIN(h_resize, type)(set, GROW_FACTOR);                                          \
    }                                                                                        \
                                                                                             \
    unsigned long hash = set->_hash(value);                                                  \
    unsigned int index = get_index(hash, set->_capacity);                                    \
                                                                                             \
    assert(set->_array);                                                                     \
    index = JOIN(h_lprobe, type)(set, value, index, false);                                  \
                                                                                             \
    if (set->_array[index].value != value)                                                   \
        set->_elements++;                                                                    \
                                                                                             \
    M_BUCKET(type) bucket = { hash, value, false };                                          \
    set->_array[index] = bucket;                                                             \
}                                                                                            \
                                                                                             \
void JOIN(erase, type)(M_SET(type)* set, type value)                                         \
{                                                                                            \
    assert(set->_elements > 0);                                                              \
                                                                                             \
    unsigned long hash = set->_hash(value);                                                  \
    unsigned int index = get_index(hash, set->_capacity);                                    \
                                                                                             \
    index = JOIN(h_lprobe, type)(set, value, index, true);                                   \
    if (index != INVALID)                                                                    \
    {                                                                                        \
        set->_array[index].value = INVALID;                                                  \
        set->_array[index].tombstone = true;                                                 \
        set->_elements--;                                                                    \
    }                                                                                        \
                                                                                             \
    float load_factor = ((float)set->_elements / (float)set->_capacity);                     \
    if (load_factor <= LOW_LOAD_FACTOR && set->_capacity > MIN_SET)                          \
        JOIN(h_resize, type)(set, SHRINK_FACTOR);                                            \
}                                                                                            \
                                                                                             \
void JOIN(clear, type)(M_SET(type) *set)                                                     \
{                                                                                            \
    free(set->_array);                                                                       \
    set->_capacity = 0;                                                                      \
    set->_elements = 0;                                                                      \
    set->_array = malloc(set->_type_size * 0);                                               \
}                                                                                            \
                                                                                             \
bool JOIN(contains, type)(M_SET(type)* set, type value)                                      \
{                                                                                            \
    if (set->_elements < 1)                                                                  \
        return false;                                                                        \
                                                                                             \
    unsigned long hash = set->_hash(value);                                                  \
    unsigned int index = get_index(hash, set->_capacity);                                    \
                                                                                             \
    index = JOIN(h_lprobe, type)(set, value, index, true);                                   \
    return index != INVALID;                                                                 \
}                                                                                            \
                                                                                             \
bool JOIN(empty, type)(M_SET(type)* set)                                                     \
{                                                                                            \
    return (set->_elements == 0);                                                            \
}                                                                                            \
                                                                                             \
size_t JOIN(size, type)(M_SET(type)* set)                                                    \
{                                                                                            \
    return set->_elements;                                                                   \
}                                                                                            \
                                                                                             \
size_t JOIN(capacity, type)(M_SET(type)* set)                                                \
{                                                                                            \
    return set->_capacity;                                                                   \
}                                                                                            \
                                                                                             \
                                                                                             \
                                                                                             \
bool JOIN(set_is_subset, type)(M_SET(type)* a, M_SET(type)* b)                               \
{                                                                                            \
    bool subset = true;                                                                      \
                                                                                             \
    if (a->size(a) > b->size(b))                                                             \
    {                                                                                        \
        subset = false;                                                                      \
    }                                                                                        \
    else                                                                                     \
    {                                                                                        \
        for (M_ITER(type) *iter = a->begin(a); iter != a->end(a); iter = a->next(a, iter))   \
        {                                                                                    \
            if (b->contains(b, iter->value))                                                 \
                continue;                                                                    \
                                                                                             \
            subset = false;                                                                  \
            break;                                                                           \
        }                                                                                    \
    }                                                                                        \
                                                                                             \
    return subset;                                                                           \
}                                                                                            \
                                                                                             \
M_SET(type) JOIN(set_union, type)(M_SET(type)* a, M_SET(type)* b)                            \
{                                                                                            \
    M_SET(type) c = set_constructor(type);                                                   \
                                                                                             \
    for (M_ITER(type) *iter = a->begin(a); iter != a->end(a); iter = a->next(a, iter))       \
    {                                                                                        \
        c.insert(&c, iter->value);                                                           \
    }                                                                                        \
    for (M_ITER(type) *iter = b->begin(b); iter != b->end(b); iter = b->next(b, iter))       \
    {                                                                                        \
        c.insert(&c, iter->value);                                                           \
    }                                                                                        \
                                                                                             \
    return c;                                                                                \
}                                                                                            \
                                                                                             \
M_SET(type) JOIN(set_difference, type)(M_SET(type)* a, M_SET(type)* b)                       \
{                                                                                            \
    M_SET(type) c = set_constructor(type);                                                   \
                                                                                             \
    for (M_ITER(type) *iter = b->begin(b); iter != b->end(b); iter = b->next(b, iter))       \
    {                                                                                        \
        if (a->contains(a, iter->value))                                                     \
            continue;                                                                        \
                                                                                             \
        c.insert(&c, iter->value);                                                           \
    }                                                                                        \
                                                                                             \
    return c;                                                                                \
}                                                                                            \
                                                                                             \
M_SET(type) JOIN(set_intersection, type)(M_SET(type)* a, M_SET(type)* b)                     \
{                                                                                            \
    M_SET(type) c = set_constructor(type);                                                   \
                                                                                             \
    for (M_ITER(type) *iter = b->begin(b); iter != b->end(b); iter = b->next(b, iter))       \
    {                                                                                        \
        if (b->contains(b, iter->value))                                                     \
            c.insert(&c, iter->value);                                                       \
    }                                                                                        \
                                                                                             \
    return c;                                                                                \
}
