#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#define GROW_FACTOR 2.f
#define SHRINK_FACTOR 0.5f
#define MIN_SET 8
#define LOW_LOAD_FACTOR 0.1f
#define HIGH_LOAD_FACTOR 0.75f

// CONSIDER: cpp-reference specifies red-black trees as typical set implementation

/* TODO: implement the below functions (if match C++ impl):
 *
 * set-theoretical functions:
 *  union(S,T): returns the union of sets S and T.
 *  intersection(S,T): returns the intersection of sets S and T.
 *  difference(S,T): returns the difference of sets S and T.
 *  subset(S,T): a predicate that tests whether the set S is a subset of set T.
 *
 * static set functions:
 *  iterate(S): returns a function that returns one more value of S at each call, in some arbitrary order.
 *  enumerate(S): returns a list containing the elements of S in some arbitrary order.
 *  build(x1,x2,…,xn,): creates a set structure with values x1,x2,...,xn.
 *  create_from(collection): creates a new set structure containing all the elements of the given collection or all the elements returned by the given iterator.
 *
 * dynamic set functions:
 *  create(): creates a new, initially empty set structure.
 *  create_with_capacity(n): creates a new set structure, initially empty but capable of holding up to n elements.
 *
 * c++ impl:
 *  swap(): swap the contents of one set with another.
 */

typedef struct SetBucket
{
    unsigned int hash;
    int value;
    bool tombstone;
} SetBucket;

typedef struct Set
{
    SetBucket* buckets;
    size_t capacity;
    size_t elements;
} Set;

static inline unsigned int get_index(unsigned int hash, size_t capacity)
{
    return hash % capacity;
}

// implementation (slightly modified) of Bernstein's dbj2 algorithm
static unsigned int dbj_hash(int item, size_t len)
{
    unsigned long hash = 5381;
    unsigned char data[len];

    memcpy(data, &item, len);

    for (size_t i = 0; i < len; i++) {
        hash = ((hash << 5) + hash) + data[i]; // hash * 33 + data[i]
    }
    return hash;
}

// replace with custom comparison for structs etc.
static inline bool compare(int a, int b)
{
    return (a == b);
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "NullDereference"
// seems to be an IDE problem
unsigned int linear_probe(Set* set, int value, unsigned int index, bool skip_tombstones)
{
    unsigned int found = -1;
    unsigned int tombstone = -1;

    while (set->buckets[index].hash != -1)
    {
        if (index >= set->capacity)
        {
            index = 0;
        }
        else if (skip_tombstones && set->buckets[index].tombstone && tombstone == -1)
        {
            tombstone = index;
        }
        else if (set->buckets[index].value == value || !skip_tombstones && set->buckets[index].tombstone)
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
    else if (tombstone != -1 && found != -1)
    {
        set->buckets[tombstone].value = set->buckets[found].value;
        set->buckets[tombstone].tombstone = false;
        set->buckets[found].value = -1;
        set->buckets[found].tombstone = true;

        found = tombstone;
    }

    return found;
}
#pragma clang diagnostic pop

void resize(Set* set, float factor)
{
    size_t size = sizeof(SetBucket) * set->capacity;
    size_t capacity = set->capacity;

    SetBucket* tmp = malloc(size);
    memcpy(tmp, set->buckets, size);
    free(set->buckets);

    set->capacity = (size_t)((float)set->capacity * factor);
    set->buckets = malloc(sizeof(SetBucket) * set->capacity);
    memset(set->buckets, -1, (size_t)((float)size * factor));

    for (int i = 0; i < capacity; i++)
    {
        SetBucket bucket = tmp[i];

        if (bucket.hash == -1 || bucket.tombstone)
            continue;

        unsigned int re_index = get_index(bucket.hash, set->capacity);
        re_index = linear_probe(set, bucket.value, re_index, false);
        set->buckets[re_index] = bucket;
    }

    free(tmp);
}

unsigned int insert(Set* set, int value, size_t len)
{
    if (set->elements == 0)
    {
        set->capacity = MIN_SET;
        free(set->buckets);
        set->buckets = malloc(sizeof(SetBucket) * set->capacity);
        memset(set->buckets, -1, set->capacity * sizeof(SetBucket));
    }
    else
    {
        float load_factor = ((float)set->elements / (float)set->capacity);
        if (load_factor >= HIGH_LOAD_FACTOR)
            resize(set, GROW_FACTOR);
    }

    unsigned int hash = dbj_hash(value, len);
    unsigned int index = get_index(hash, set->capacity);

    index = linear_probe(set, value, index, false);

    if (set->buckets[index].value != value)
        set->elements++;

    SetBucket bucket = {hash, value, false };
    set->buckets[index] = bucket;

    return index;
}

void erase(Set* set, int value, size_t len)
{
    if (set->elements == 0)
    {
        set->capacity = MIN_SET;
        free(set->buckets);
        set->buckets = malloc(sizeof(SetBucket) * set->capacity);
        memset(set->buckets, -1, set->capacity * sizeof(SetBucket));
    }
    else
    {
        float load_factor = ((float)set->elements / (float)set->capacity);
        if (load_factor <= LOW_LOAD_FACTOR)
            resize(set, SHRINK_FACTOR);
    }

    unsigned int hash = dbj_hash(value, len);
    unsigned int index = get_index(hash, set->capacity);

    index = linear_probe(set, value, index, true);
    if (index != -1)
    {
        set->buckets[index].value = -1;
        set->buckets[index].tombstone = true;
        set->elements--;
    }
}

bool find(Set* set, int value, size_t len)
{
    unsigned int hash = dbj_hash(value, len);
    unsigned int index = get_index(hash, set->capacity);

    return linear_probe(set, value, index, true);
}

bool contains(Set* set, int value, size_t len)
{
    unsigned int hash = dbj_hash(value, len);
    unsigned int index = get_index(hash, set->capacity);

    index = linear_probe(set, value, index, true);
    return index != -1;
}

inline bool empty(Set* set)
{
    return (set->elements == 0);
}

inline size_t size(Set* set)
{
    return set->elements;
}

inline size_t capacity(Set* set)
{
    return set->capacity;
}

void clear(Set* set)
{
    free(set->buckets);
    set->capacity = 0;
    set->elements = 0;
    malloc(sizeof(SetBucket) * 0);
}


