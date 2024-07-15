#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#define set_constructor(type)                                                 \
    set_constructor_custom(type, compare_general_##type, hash_general_##type) \

#define set_constructor_custom(type, cmp, hsh)                              \
{                                                                           \
    ._array = malloc(sizeof(SetBucket_##type) * 0),                         \
    ._capacity = 0, ._elements = 0, ._type_size = sizeof(SetBucket_##type), \
    ._cmp = cmp, ._hash = hsh,                                              \
    .begin = begin_##type, .next = next_##type, .end = end_##type,          \
    .insert = insert_##type, .erase = erase_##type, .clear = clear_##type,  \
    .contains = contains_##type, .empty = empty_##type,                     \
    .size = size_##type, .capacity = capacity_##type                        \
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
bool compare_general_##type(type a, type b)                                                  \
{                                                                                            \
    return a == b;                                                                           \
}                                                                                            \
                                                                                             \
bool compare_string_##type(const char* a, const char* b)                                     \
{                                                                                            \
    return !strcmp(a, b);                                                                    \
}                                                                                            \
                                                                                             \
unsigned long hash_general_##type(type item)                                                 \
{                                                                                            \
    size_t len = sizeof(item);                                                               \
    unsigned char data[len];                                                                 \
    memcpy(data, &item, len);                                                                \
                                                                                             \
    return djb2(data, len);                                                                  \
}                                                                                            \
                                                                                             \
unsigned long hash_string_##type(const char* item)                                           \
{                                                                                            \
    size_t len = strlen(item);                                                               \
    unsigned char data[len];                                                                 \
    memcpy(data, &item, len);                                                                \
                                                                                             \
    return djb2(data, len);                                                                  \
}                                                                                            \
                                                                                             \
typedef struct SetBucket_##type                                                              \
{                                                                                            \
    unsigned long hash;                                                                      \
    type value;                                                                              \
    bool tombstone;                                                                          \
} SetBucket_##type, SetIter_##type;                                                          \
                                                                                             \
typedef struct Set_##type                                                                    \
{                                                                                            \
    SetBucket_##type* _array;                                                                \
    size_t _capacity;                                                                        \
    size_t _elements;                                                                        \
    size_t _type_size;                                                                       \
                                                                                             \
    bool (*_cmp)(type, type);                                                                \
    unsigned long (*_hash)(type);                                                            \
                                                                                             \
    SetIter_##type *(*begin)(struct Set_int*);                                               \
    SetIter_##type *(*next)(struct Set_int*, SetIter_##type*);                               \
    SetIter_##type *(*end)(struct Set_int*);                                                 \
                                                                                             \
    void (*insert)(struct Set_##type*, type);                                                \
    void (*erase)(struct Set_##type*, type);                                                 \
    bool (*contains)(struct Set_##type*, type);                                              \
    bool (*empty)(struct Set_##type*);                                                       \
    size_t (*size)(struct Set_##type*);                                                      \
    size_t (*capacity)(struct Set_##type*);                                                  \
    void (*clear)(struct Set_##type*);                                                       \
} Set_##type;                                                                                \
                                                                                             \
SetIter_##type *begin_int(Set_##type* set)                                                   \
{                                                                                            \
    SetIter_##type *iter = set->_array;                                                      \
    while (                                                                                  \
        iter < (set->_array + set->_capacity) && (iter->hash == -1 || iter->tombstone)       \
    ) ++iter;                                                                                \
    return iter;                                                                             \
}                                                                                            \
                                                                                             \
SetIter_##type *next_int(Set_##type* set, SetIter_##type* iter)                              \
{                                                                                            \
    do {                                                                                     \
        ++iter;                                                                              \
    } while (                                                                                \
        iter < (set->_array + set->_capacity) && (iter->hash == -1 || iter->tombstone)       \
    );                                                                                       \
    return iter;                                                                             \
}                                                                                            \
                                                                                             \
SetIter_##type *end_int(Set_int* set)                                                        \
{                                                                                            \
    return set->_array + set->_capacity;                                                     \
}                                                                                            \
                                                                                             \
unsigned int                                                                                 \
h_lprobe_##type(Set_##type *set, int value, unsigned int index, bool skip_tombstones)        \
{                                                                                            \
    assert(set && set->_array);                                                              \
                                                                                             \
    unsigned int found = -1;                                                                 \
    unsigned int tombstone = -1;                                                             \
    unsigned int count = 1;                                                                  \
                                                                                             \
    while (found == -1)                                                                      \
    {                                                                                        \
        SetBucket_##type bucket = set->_array[index];                                        \
                                                                                             \
        if (skip_tombstones && tombstone == -1 &&                                            \
            bucket.hash != -1 && bucket.tombstone)                                           \
            tombstone = index;                                                               \
        else if (set->_cmp(bucket.value, value) || (!skip_tombstones &&                      \
            (bucket.hash == -1 || bucket.tombstone)))                                        \
            found = index;                                                                   \
        else if (skip_tombstones && bucket.hash == -1)                                       \
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
        set->_array[found].value = -1;                                                       \
        set->_array[found].tombstone = true;                                                 \
                                                                                             \
        found = tombstone;                                                                   \
    }                                                                                        \
                                                                                             \
    return found;                                                                            \
}                                                                                            \
                                                                                             \
void h_resize_##type(Set_##type *set, float factor)                                          \
{                                                                                            \
    assert(set->_array);                                                                     \
                                                                                             \
    size_t size = set->_type_size * set->_capacity;                                          \
    size_t capacity = set->_capacity;                                                        \
                                                                                             \
    SetBucket_##type* tmp = malloc(size);                                                    \
    memcpy(tmp, set->_array, size);                                                          \
    free(set->_array);                                                                       \
                                                                                             \
    set->_capacity = (size_t)((float)set->_capacity * factor);                               \
    set->_array = malloc(set->_type_size * set->_capacity);                                  \
    assert(set->_array);                                                                     \
    memset(set->_array, -1, (size_t)((float)size * factor));                                 \
                                                                                             \
    SetBucket_##type bucket;                                                                 \
    for (int i = 0; i < capacity; i++)                                                       \
    {                                                                                        \
        bucket = tmp[i];                                                                     \
                                                                                             \
        if (bucket.hash == -1 || bucket.tombstone)                                           \
            continue;                                                                        \
                                                                                             \
        unsigned int re_index = get_index(bucket.hash, set->_capacity);                      \
        re_index = h_lprobe_##type(set, bucket.value, re_index, false);                      \
        set->_array[re_index] = bucket;                                                      \
    }                                                                                        \
                                                                                             \
    free(tmp);                                                                               \
}                                                                                            \
                                                                                             \
void insert_##type(Set_##type* set, type value)                                              \
{                                                                                            \
    if (set->_elements == 0)                                                                 \
    {                                                                                        \
        set->_capacity = 8;                                                                  \
        free(set->_array);                                                                   \
        set->_array = malloc(set->_type_size * set->_capacity);                              \
        memset(set->_array, -1, set->_type_size * set->_capacity);                           \
    }                                                                                        \
    else                                                                                     \
    {                                                                                        \
        float load_factor = ((float)set->_elements / (float)set->_capacity);                 \
        if (load_factor >= 0.75f)                                                            \
            h_resize_##type(set, 2.f);                                                       \
    }                                                                                        \
                                                                                             \
    unsigned long hash = set->_hash(value);                                                  \
    unsigned int index = get_index(hash, set->_capacity);                                    \
                                                                                             \
    assert(set->_array);                                                                     \
    index = h_lprobe_##type(set, value, index, false);                                       \
                                                                                             \
    if (set->_array[index].value != value)                                                   \
        set->_elements++;                                                                    \
                                                                                             \
    SetBucket_##type bucket = { hash, value, false };                                        \
    set->_array[index] = bucket;                                                             \
}                                                                                            \
                                                                                             \
void erase_##type(Set_##type* set, type value)                                               \
{                                                                                            \
    assert(set->_elements > 0);                                                              \
                                                                                             \
    unsigned long hash = set->_hash(value);                                                  \
    unsigned int index = get_index(hash, set->_capacity);                                    \
                                                                                             \
    index = h_lprobe_##type(set, value, index, true);                                        \
    if (index != -1)                                                                         \
    {                                                                                        \
        set->_array[index].value = -1;                                                       \
        set->_array[index].tombstone = true;                                                 \
        set->_elements--;                                                                    \
    }                                                                                        \
                                                                                             \
    float load_factor = ((float)set->_elements / (float)set->_capacity);                     \
    if (load_factor <= 0.1f && set->_capacity > 8)                                           \
        h_resize_##type(set, 0.5f);                                                          \
}                                                                                            \
                                                                                             \
void clear_##type(Set_##type *set)                                                           \
{                                                                                            \
    free(set->_array);                                                                       \
    set->_capacity = 0;                                                                      \
    set->_elements = 0;                                                                      \
    set->_array = malloc(set->_type_size * 0);                                               \
}                                                                                            \
                                                                                             \
bool contains_##type(Set_##type* set, type value)                                            \
{                                                                                            \
    if (set->_elements < 1)                                                                  \
        return false;                                                                        \
                                                                                             \
    unsigned long hash = set->_hash(value);                                                  \
    unsigned int index = get_index(hash, set->_capacity);                                    \
                                                                                             \
    index = h_lprobe_##type(set, value, index, true);                                        \
    return index != -1;                                                                      \
}                                                                                            \
                                                                                             \
bool empty_##type(Set_##type* set)                                                           \
{                                                                                            \
    return (set->_elements == 0);                                                            \
}                                                                                            \
                                                                                             \
size_t size_##type(Set_##type* set)                                                          \
{                                                                                            \
    return set->_elements;                                                                   \
}                                                                                            \
                                                                                             \
size_t capacity_##type(Set_##type* set)                                                      \
{                                                                                            \
    return set->_capacity;                                                                   \
}                                                                                            \
                                                                                             \
                                                                                             \
                                                                                             \
bool set_is_subset_##type(Set_##type* a, Set_##type* b)                                      \
{                                                                                            \
    bool subset = true;                                                                      \
                                                                                             \
    if (a->size(a) > b->size(b))                                                             \
    {                                                                                        \
        subset = false;                                                                      \
    }                                                                                        \
    else                                                                                     \
    {                                                                                        \
        for (SetIter_##type *iter = a->begin(a); iter != a->end(a); iter = a->next(a, iter)) \
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
Set_##type set_union_##type(Set_##type* a, Set_##type* b)                                    \
{                                                                                            \
    Set_##type c = set_constructor(type);                                                    \
                                                                                             \
    for (SetIter_##type *iter = a->begin(a); iter != a->end(a); iter = a->next(a, iter))     \
    {                                                                                        \
        c.insert(&c, iter->value);                                                           \
    }                                                                                        \
    for (SetIter_##type *iter = b->begin(b); iter != b->end(b); iter = b->next(b, iter))     \
    {                                                                                        \
        c.insert(&c, iter->value);                                                           \
    }                                                                                        \
                                                                                             \
    return c;                                                                                \
}                                                                                            \
                                                                                             \
Set_##type set_difference_##type(Set_##type* a, Set_##type* b)                               \
{                                                                                            \
    Set_##type c = set_constructor(type);                                                    \
                                                                                             \
    for (SetIter_##type *iter = b->begin(b); iter != b->end(b); iter = b->next(b, iter))     \
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
Set_##type set_intersection_##type(Set_##type* a, Set_##type* b)                             \
{                                                                                            \
    Set_##type c = set_constructor(type);                                                    \
                                                                                             \
    for (SetIter_##type *iter = b->begin(b); iter != b->end(b); iter = b->next(b, iter))     \
    {                                                                                        \
        if (b->contains(b, iter->value))                                                     \
            c.insert(&c, iter->value);                                                       \
    }                                                                                        \
                                                                                             \
    return c;                                                                                \
}
