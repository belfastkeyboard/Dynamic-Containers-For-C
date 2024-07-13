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
#endif

#define set_constructor(type)                                                \
{                                                                            \
    ._array = malloc(sizeof(SetBucket) * 0),                                 \
    ._capacity = 0, ._elements = 0, ._type_size = sizeof(type),              \
    ._cmp = compare_general, ._hash = hash_general,                          \
    .insert = insert, .erase = erase, .clear = clear,                        \
    .contains = contains, .empty = empty, .size = size, .capacity = capacity \
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
bool compare_general(int a, int b)
{
    return a == b;
}
bool compare_string(const char* a, const char* b)
{
    return !strcmp(a, b);
}
unsigned long hash_general(int item)
{
    size_t len = sizeof(item);
    unsigned char data[len];
    memcpy(data, &item, len);

    return djb2(data, len);
}
unsigned long hash_string(const char* item)
{
    size_t len = strlen(item);
    unsigned char data[len];
    memcpy(data, &item, len);

    return djb2(data, len);
}

#define SET(type) type                                                                            \
                                                                                                  \
typedef struct JOIN(SetBucket, type)                                                              \
{                                                                                                 \
    unsigned long hash;                                                                           \
    int value;                                                                                    \
    bool tombstone;                                                                               \
} JOIN(SetBucket, type);                                                                          \
                                                                                                  \
typedef struct JOIN(Set, type)                                                                    \
{                                                                                                 \
    SetBucket* _array;                                                                            \
    size_t _capacity;                                                                             \
    size_t _elements;                                                                             \
    size_t _type_size;                                                                            \
                                                                                                  \
    // plug-n-play function pointers                                                              \
    bool (*_cmp)(int, int);                                                                       \
    unsigned long (*_hash)(int);                                                                  \
                                                                                                  \
    void (*insert)(struct Set*, int);                                                             \
    void (*erase)(struct Set*, int);                                                              \
    bool (*contains)(struct Set*, int);                                                           \
    bool (*empty)(struct Set*);                                                                   \
    size_t (*size)(struct Set*);                                                                  \
    size_t (*capacity)(struct Set*);                                                              \
    void (*clear)(struct Set*);                                                                   \
} JOIN(Set, type);                                                                                \
                                                                                                  \
typedef struct JOIN(SetIterator, type)                                                            \
{                                                                                                 \
    Set* set;                                                                                     \
    int index;                                                                                    \
} JOIN(SetIterator, type);                                                                        \
                                                                                                  \
bool JOIN(h_iterate, type)(JOIN(SetIterator, type) *iterator, JOIN(SetBucket, type) *bucket)      \
{                                                                                                 \
    assert(iterator != NULL);                                                                     \
                                                                                                  \
    for (int i = iterator->index; i < iterator->set->_capacity; i++)                              \
    {                                                                                             \
        *bucket = iterator->set->_array[i];                                                       \
                                                                                                  \
        if (bucket->hash == INVALID || bucket->tombstone)                                         \
            continue;                                                                             \
                                                                                                  \
        iterator->index++;                                                                        \
        return true;                                                                              \
    }                                                                                             \
                                                                                                  \
    return false;                                                                                 \
}                                                                                                 \
                                                                                                  \
unsigned int                                                                                      \
JOIN(h_lprobe, type)(JOIN(Set, type)* set, int value, unsigned int index, bool skip_tombstones)   \
{                                                                                                 \
    assert(set && set->_array);                                                                   \
                                                                                                  \
    unsigned int found = -1;                                                                      \
    unsigned int tombstone = -1;                                                                  \
                                                                                                  \
    while (set->_array[index].hash != -1)                                                         \
    {                                                                                             \
        if (index >= set->_capacity)                                                              \
        {                                                                                         \
            index = 0;                                                                            \
        }                                                                                         \
        else if (skip_tombstones && set->_array[index].tombstone && tombstone == INVALID)         \
        {                                                  // ????? tombstone == INVALID ????     \
            tombstone = index;                                                                    \
        }                                                                                         \
        else if (set->_cmp(set->_array[index].value, value)  ||                                   \
            !skip_tombstones && set->_array[index].tombstone)                                     \
        {                                                                                         \
            found = index;                                                                        \
            break;                                                                                \
        }                                                                                         \
        else                                                                                      \
        {                                                                                         \
            index++;                                                                              \
        }                                                                                         \
    }                                                                                             \
                                                                                                  \
    if (!skip_tombstones)                                                                         \
    {                                                                                             \
        found = index;                                                                            \
    }                                                                                             \
    else if (tombstone != INVALID && found != INVALID)                                            \
    {                                                                                             \
        set->_array[tombstone].value = set->_array[found].value;                                  \
        set->_array[tombstone].tombstone = false;                                                 \
        set->_array[found].value = INVALID;                                                       \
        set->_array[found].tombstone = true;                                                      \
                                                                                                  \
        found = tombstone;                                                                        \
    }                                                                                             \
                                                                                                  \
    return found;                                                                                 \
}                                                                                                 \
                                                                                                  \
void h_resize(JOIN(Set, type)* set, float factor)                                                 \
{                                                                                                 \
    size_t size = sizeof(SetBucket) * set->_capacity;                                             \
    size_t capacity = set->_capacity;                                                             \
                                                                                                  \
    SetBucket* tmp = malloc(size);                                                                \
    memcpy(tmp, set->_array, size);                                                               \
    free(set->_array);                                                                            \
                                                                                                  \
    set->_capacity = (size_t)((float)set->_capacity * factor);                                    \
    set->_array = malloc(sizeof(SetBucket) * set->_capacity);                                     \
    memset(set->_array, INVALID, (size_t)((float)size * factor));                                 \
                                                                                                  \
    for (int i = 0; i < capacity; i++)                                                            \
    {                                                                                             \
        SetBucket bucket = tmp[i];                                                                \
                                                                                                  \
        if (bucket.hash == INVALID || bucket.tombstone)                                           \
            continue;                                                                             \
                                                                                                  \
        unsigned int re_index = get_index(bucket.hash, set->_capacity);                           \
        re_index = h_lprobe(set, bucket.value, re_index, false);                                  \
        set->_array[re_index] = bucket;                                                           \
    }                                                                                             \
                                                                                                  \
    free(tmp);                                                                                    \
}                                                                                                 \
                                                                                                  \
// member functions                                                                               \
void JOIN(insert, type)(JOIN(Set, type)* set, int value)                                          \
{                                                                                                 \
    if (set->_elements == 0)                                                                      \
    {                                                                                             \
        set->_capacity = MIN_SET;                                                                 \
        free(set->_array);                                                                        \
        set->_array = malloc(sizeof(JOIN(SetBucket, type)) * set->_capacity);                     \
        memset(set->_array, -1, set->_capacity * sizeof(JOIN(SetBucket, type)));                  \
    }                                                                                             \
    else                                                                                          \
    {                                                                                             \
        float load_factor = ((float)set->_elements / (float)set->_capacity);                      \
        if (load_factor >= HIGH_LOAD_FACTOR)                                                      \
            JOIN(h_resize, type)(set, GROW_FACTOR);                                               \
    }                                                                                             \
                                                                                                  \
    unsigned long hash = set->_hash(value);                                                       \
    unsigned int index = get_index(hash, set->_capacity);                                         \
                                                                                                  \
    index = JOIN(h_lprobe, type)(set, value, index, false);                                       \
                                                                                                  \
    if (set->_array[index].value != value)                                                        \
        set->_elements++;                                                                         \
                                                                                                  \
    JOIN(SetBucket, type) bucket = {hash, value, false };                                         \
    set->_array[index] = JOIN(bucket, type);                                                      \
}                                                                                                 \
void JOIN(erase, type)(JOIN(Set, type)* set, int value)                                           \
{                                                                                                 \
    assert(set->_elements > 0);                                                                   \
                                                                                                  \
    unsigned long hash = set->_hash(value);                                                       \
    unsigned int index = get_index(hash, set->_capacity);                                         \
                                                                                                  \
    index = h_lprobe(set, value, index, true);                                                    \
    if (index != -1)                                                                              \
    {                                                                                             \
        set->_array[index].value = -1;                                                            \
        set->_array[index].tombstone = true;                                                      \
        set->_elements--;                                                                         \
    }                                                                                             \
                                                                                                  \
    float load_factor = ((float)set->_elements / (float)set->_capacity);                          \
    if (load_factor <= LOW_LOAD_FACTOR)                                                           \
        h_resize(set, SHRINK_FACTOR);                                                             \
}                                                                                                 \
void JOIN(clear, type)(JOIN(Set, type)* set)                                                      \
{                                                                                                 \
    free(set->_array);                                                                            \
    set->_capacity = 0;                                                                           \
    set->_elements = 0;                                                                           \
    set->_array = malloc(sizeof(JOIN(SetBucket, type)) * 0);                                      \
}                                                                                                 \
                                                                                                  \
bool JOIN(contains, type)(JOIN(Set, type)* set, int value)                                        \
{                                                                                                 \
    unsigned long hash = set->_hash(value);                                                       \
    unsigned int index = get_index(hash, set->_capacity);                                         \
                                                                                                  \
    index = JOIN(h_lprobe, type)(set, value, index, true);                                        \
    return index != -1;                                                                           \
}                                                                                                 \
bool JOIN(empty, type)(JOIN(Set, type)* set)                                                      \
{                                                                                                 \
    return (set->_elements == 0);                                                                 \
}                                                                                                 \
size_t JOIN(size, type)(JOIN(Set, type)* set)                                                     \
{                                                                                                 \
    return set->_elements;                                                                        \
}                                                                                                 \
size_t JOIN(capacity, type)(JOIN(Set, type)* set)                                                 \
{                                                                                                 \
    return set->_capacity;                                                                        \
}                                                                                                 \
                                                                                                  \
                                                                                                  \
                                                                                                  \
// set-theoretical functions:                                                                     \
bool JOIN(set_is_subset, type)(JOIN(Set, type)* a, JOIN(Set, type)* b)                            \
{                                                                                                 \
    bool subset = true;                                                                           \
                                                                                                  \
    if (a->size() > b->size())                                                                    \
    {                                                                                             \
        subset = false;                                                                           \
    }                                                                                             \
    else                                                                                          \
    {                                                                                             \
        JOIN(SetBucket, type) bucket = { 0 };                                                     \
        JOIN(SetIterator, type) iter = { a, 0 };                                                  \
                                                                                                  \
        while(JOIN(h_iterate, type)(&iter, &bucket))                                              \
        {                                                                                         \
            if (b.contains(b, bucket.value))                                                      \
                continue;                                                                         \
                                                                                                  \
            subset = false;                                                                       \
            break;                                                                                \
        }                                                                                         \
    }                                                                                             \
                                                                                                  \
    return subset;                                                                                \
}                                                                                                 \
                                                                                                  \
JOIN(Set, type) JOIN(set_union, type)((JOIN(Set, type)* a, JOIN(Set, type)* b)                    \
{                                                                                                 \
    JOIN(Set, type) c = set_constructor(int);                                                     \
                                                                                                  \
    JOIN(SetBucket, type) bucket = { 0 };                                                         \
    JOIN(SetIterator, type) iter = { a, 0 };                                                      \
    while(h_iterate(&iter, &bucket))                                                              \
    {                                                                                             \
        c.insert(&c, bucket.value);                                                               \
    }                                                                                             \
    iter.set = b;                                                                                 \
    iter.index = 0;                                                                               \
    while(h_iterate(&iter, &bucket))                                                              \
    {                                                                                             \
        c.insert(&c, bucket.value);                                                               \
    }                                                                                             \
                                                                                                  \
    return c;                                                                                     \
}                                                                                                 \
                                                                                                  \
JOIN(Set, type) JOIN(set_difference, type)(JOIN(Set, type)* a, JOIN(Set, type)* b)                \
{                                                                                                 \
    JOIN(Set, type) c = set_constructor(int);                                                     \
                                                                                                  \
    JOIN(SetBucket, type) bucket = { 0 };                                                         \
    JOIN(SetIterator, type) iter = { b, 0 };                                                      \
    while(h_iterate(&iter, &bucket))                                                              \
    {                                                                                             \
        if (a.contains(a, bucket.value))                                                          \
            continue;                                                                             \
                                                                                                  \
        c.insert(&c, bucket.value);                                                               \
    }                                                                                             \
                                                                                                  \
    return c;                                                                                     \
}                                                                                                 \
                                                                                                  \
JOIN(Set, type) JOIN(set_intersection, type)(JOIN(Set, type)* a, JOIN(Set, type)* b)              \
{                                                                                                 \
    JOIN(Set, type) c = set_constructor(int);                                                     \
                                                                                                  \
    JOIN(SetBucket, type) bucket = { 0 };                                                         \
    JOIN(SetIterator, type) iter = { a, 0 };                                                      \
    while(h_iterate(&iter, &bucket))                                                              \
    {                                                                                             \
        if (b.contains(b, bucket.value))                                                          \
            c.insert(&c, bucket.value);                                                           \
    }                                                                                             \
                                                                                                  \
    return c;                                                                                     \
}                                                                                                 \


#undef MIN_SET
#undef LOW_LOAD_FACTOR
#undef HIGH_LOAD_FACTOR
#undef INVALID
