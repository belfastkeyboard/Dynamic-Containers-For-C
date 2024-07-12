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
inline unsigned int get_index(unsigned long hash, size_t capacity)
{
    return hash % capacity;
}

// TODO: make generic

typedef struct SetBucket
{
    unsigned long hash;
    int value;
    bool tombstone;
} SetBucket;
typedef struct Set
{
    SetBucket* _array;
    size_t _capacity;
    size_t _elements;
    size_t _type_size;

    // these function pointers are plug-n-play
    // for structs or other complex comparisons/hashes
    // they can be substituted with custom functions
    bool (*_cmp)(int, int);
    unsigned long (*_hash)(int);

    void (*insert)(struct Set*, int);
    void (*erase)(struct Set*, int);
    bool (*contains)(struct Set*, int);
    bool (*empty)(struct Set*);
    size_t (*size)(struct Set*);
    size_t (*capacity)(struct Set*);
    void (*clear)(struct Set*);
} Set;
typedef struct SetIterator
{
    Set* set;
    int index;
} SetIterator;


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


// helper functions
int helper_iterate(SetIterator *iterator, SetBucket *bucket)
{
    assert(iterator != NULL);

    for (int i = iterator->index; i < iterator->set->_capacity; i++)
    {
        *bucket = iterator->set->_array[i];

        if (bucket->hash == INVALID || bucket->tombstone)
            continue;

        iterator->index++;
        return true;
    }

    return false;
}
unsigned int helper_linear_probe(Set* set, int value, unsigned int index, bool skip_tombstones)
{
    assert(set && set->_array);

    unsigned int found = -1;
    unsigned int tombstone = -1;

    while (set->_array[index].hash != -1)
    {
        if (index >= set->_capacity)
        {
            index = 0;
        }
        else if (skip_tombstones && set->_array[index].tombstone && tombstone == INVALID) // ????? tombstone == INVALID ????
        {
            tombstone = index;
        }
        else if (set->_cmp(set->_array[index].value, value)  || !skip_tombstones && set->_array[index].tombstone)
        {
            found = index;
            break;
        }
        else
        {
            index++;
        }
    }

    if (!skip_tombstones)
    {
        found = index;
    }
    else if (tombstone != INVALID && found != INVALID)
    {
        set->_array[tombstone].value = set->_array[found].value;
        set->_array[tombstone].tombstone = false;
        set->_array[found].value = INVALID;
        set->_array[found].tombstone = true;

        found = tombstone;
    }

    return found;
}
void helper_resize(Set* set, float factor)
{
    size_t size = sizeof(SetBucket) * set->_capacity;
    size_t capacity = set->_capacity;

    SetBucket* tmp = malloc(size);
    memcpy(tmp, set->_array, size);
    free(set->_array);

    set->_capacity = (size_t)((float)set->_capacity * factor);
    set->_array = malloc(sizeof(SetBucket) * set->_capacity);
    memset(set->_array, INVALID, (size_t)((float)size * factor));

    for (int i = 0; i < capacity; i++)
    {
        SetBucket bucket = tmp[i];

        if (bucket.hash == INVALID || bucket.tombstone)
            continue;

        unsigned int re_index = get_index(bucket.hash, set->_capacity);
        re_index = helper_linear_probe(set, bucket.value, re_index, false);
        set->_array[re_index] = bucket;
    }

    free(tmp);
}



// member functions
void insert(Set* set, int value)
{
    if (set->_elements == 0)
    {
        set->_capacity = MIN_SET;
        free(set->_array);
        set->_array = malloc(sizeof(SetBucket) * set->_capacity);
        memset(set->_array, -1, set->_capacity * sizeof(SetBucket));
    }
    else
    {
        float load_factor = ((float)set->_elements / (float)set->_capacity);
        if (load_factor >= HIGH_LOAD_FACTOR)
            helper_resize(set, GROW_FACTOR);
    }

    unsigned long hash = set->_hash(value);
    unsigned int index = get_index(hash, set->_capacity);

    index = helper_linear_probe(set, value, index, false);

    if (set->_array[index].value != value)
        set->_elements++;

    SetBucket bucket = {hash, value, false };
    set->_array[index] = bucket;
}
void erase(Set* set, int value)
{
    assert(set->_elements > 0);

    unsigned long hash = set->_hash(value);
    unsigned int index = get_index(hash, set->_capacity);

    index = helper_linear_probe(set, value, index, true);
    if (index != -1)
    {
        set->_array[index].value = -1;
        set->_array[index].tombstone = true;
        set->_elements--;
    }

    float load_factor = ((float)set->_elements / (float)set->_capacity);
    if (load_factor <= LOW_LOAD_FACTOR)
        helper_resize(set, SHRINK_FACTOR);
}
void clear(Set* set)
{
    free(set->_array);
    set->_capacity = 0;
    set->_elements = 0;
    set->_array = malloc(sizeof(SetBucket) * 0);
}

bool contains(Set* set, int value)
{
    unsigned long hash = set->_hash(value);
    unsigned int index = get_index(hash, set->_capacity);

    index = helper_linear_probe(set, value, index, true);
    return index != -1;
}
bool empty(Set* set)
{
    return (set->_elements == 0);
}
size_t size(Set* set)
{
    return set->_elements;
}
size_t capacity(Set* set)
{
    return set->_capacity;
}



// set-theoretical functions:
bool set_is_subset(Set* a, Set* b)
{
    bool subset = true;

    if (a->_elements > b->_elements)
    {
        subset = false;
    }
    else
    {
        SetBucket bucket = { 0 };
        SetIterator iter = { a, 0 };

        while(helper_iterate(&iter, &bucket))
        {
            if (contains(b, bucket.value))
                continue;

            subset = false;
            break;
        }
    }

    return subset;
}
Set set_union(Set* a, Set* b)
{
    Set c = set_constructor(int);

    SetBucket bucket = { 0 };
    SetIterator iter = { a, 0 };
    while(helper_iterate(&iter, &bucket))
    {
        insert(&c, bucket.value);
    }
    iter.set = b;
    iter.index = 0;
    while(helper_iterate(&iter, &bucket))
    {
        insert(&c, bucket.value);
    }

    return c;
}
Set set_difference(Set* a, Set* b)
{
    Set c = set_constructor(int);

    SetBucket bucket = { 0 };
    SetIterator iter = { b, 0 };
    while(helper_iterate(&iter, &bucket))
    {
        if (contains(a, bucket.value))
            continue;

        c.insert(&c, bucket.value);
    }

    return c;
}
Set set_intersection(Set* a, Set* b)
{
    Set c = set_constructor(int);

    SetBucket bucket = { 0 };
    SetIterator iter = { a, 0 };
    while(helper_iterate(&iter, &bucket))
    {
        if (contains(b, bucket.value))
            insert(&c, bucket.value);
    }

    return c;
}


#undef MIN_SET
#undef LOW_LOAD_FACTOR
#undef HIGH_LOAD_FACTOR
#undef INVALID
