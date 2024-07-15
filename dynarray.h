// Dynamic array implementation in C based on C++ vector implementation

// TODO: test

/*  HOW TO USE:

    Call ARRAY(type) with the desired type, multiple types can be used.
    Call constructor_array(type) to define attributes and function pointers.
    Call destructor(arr) in order to clean up.
    If array goes out of scope without destructor being called, a memory leak will occur.

    example:

    ARRAY(int)

    int main(void)
    {
        array_int arr = constructor_array(int);

        arr.push_back_int(&arr, 1);
        arr.push_back_int(&arr, 2);

        destructor(arr);
        
        return 0;
    }

    Do not manually modify: _elements, _capacity, _type_size, or _array.
    Use function pointers to do so.

*/

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include <stdbool.h>

#undef GROW_FACTOR
#undef ARRAY_MAIN

#define GROW_FACTOR 2
#define ARRAY_MIN 1

#define constructor_array(type)                                                                    \
{                                                                                                  \
    ._elements = 0, ._capacity = 0, ._type_size = sizeof(type), ._array = calloc(0, sizeof(type)), \
    .push_back = array_push_##type, .insert = array_insert_##type,                                 \
    .pop_back = array_pop_back_##type, .erase = array_erase_##type, .clear = array_clear_##type,   \
    .front = array_front_##type, .back = array_back_##type, .get = array_get_##type,               \
    .empty = array_empty_##type, .size = array_size_##type,                                        \
    .reserve = array_reserve_##type, .shrink = array_shrink_##type                                 \
}

#ifndef destructor
    #define destructor(item) \
        free(item._array);   \
        item._array = NULL;  \
        item._elements = 0;  \
        item._capacity = 0;  \
        item._type_size = 0
#endif

#define ARRAY(type)                                       \
typedef struct array_##type                               \
{                                                         \
    type*  _array;                                        \
    size_t _elements;                                     \
    size_t _capacity;                                     \
    size_t _type_size;                                    \
    void   (*push_back)(struct array_##type*, type);      \
    void   (*insert)(struct array_##type*, type, size_t); \
    void   (*pop_back)(struct array_##type*);             \
    size_t (*erase)(struct array_##type*, size_t);        \
    type   (*front)(struct array_##type*);                \
    type   (*back)(struct array_##type*);                 \
    type   (*get)(struct array_##type*, size_t);          \
    bool   (*empty)(struct array_##type*);                \
    size_t (*size)(struct array_##type*);                 \
    void   (*clear)(struct array_##type*);                \
    void   (*reserve)(struct array_##type*, size_t);      \
    void   (*shrink)(struct array_##type*);               \
} array_##type;                                                             \
                                                                            \
void array_push_##type(struct array_##type* arr, type elem)                 \
{                                                                           \
    if (arr->_elements >= arr->_capacity)                                   \
    {                                                                       \
        arr->_capacity = (arr->_capacity > 0) ?                             \
            arr->_capacity * GROW_FACTOR : ARRAY_MIN;                       \
                                                                            \
        type* tmp = realloc(arr->_array, arr->_type_size * arr->_capacity); \
        assert(tmp != NULL);                                                \
        arr->_array = tmp;                                                  \
    }                                                                       \
    arr->_array[arr->_elements] = elem;                                     \
    arr->_elements++;                                                       \
}                                                                           \
                                                                            \
void array_insert_##type(struct array_##type* arr, type elem, size_t index) \
{                                                                           \
    if (arr->_elements >= arr->_capacity)                                   \
    {                                                                       \
        arr->_capacity = (arr->_capacity > 0) ?                             \
            arr->_capacity * GROW_FACTOR : ARRAY_MIN;                       \
        type* tmp = realloc(arr->_array, arr->_type_size * arr->_capacity); \
                                                                            \
        assert(tmp != NULL);                                                \
                                                                            \
        arr->_array = tmp;                                                  \
    }                                                                       \
                                                                            \
    assert(index <= arr->_elements);                                        \
                                                                            \
    if (index == arr->_elements)                                            \
    {                                                                       \
        arr->_array[arr->_elements] = elem;                                 \
        arr->_elements++;                                                   \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        type *source = &arr->_array[index];                                 \
        type *destination = &arr->_array[index + 1];                        \
        size_t amount = (arr->_elements - index) * arr->_type_size;         \
        memmove(destination, source, amount);                               \
        arr->_array[index] = elem;                                          \
        arr->_elements++;                                                   \
    }                                                                       \
}                                                                           \
                                                                            \
void array_pop_back_##type(struct array_##type* arr)                        \
{                                                                           \
    assert(arr->_elements > 0);                                             \
    arr->_elements--;                                                       \
}                                                                           \
                                                                            \
size_t array_erase_##type(struct array_##type* arr, size_t index)           \
{                                                                           \
    assert(index <= arr->_elements);                                        \
    if (index == arr->_elements - 1)                                        \
    {                                                                       \
        arr->_elements--;                                                   \
        return index - 1;                                                   \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        type *source = &arr->_array[index + 1];                             \
        type *destination = &arr->_array[index];                            \
        size_t amount = (arr->_elements - index) * arr->_type_size;         \
        memmove(destination, source, amount);                               \
        arr->_elements--;                                                   \
        return index - 1;                                                   \
    }                                                                       \
}                                                                           \
                                                                            \
type array_front_##type(struct array_##type* arr)                           \
{                                                                           \
    assert(arr->_elements > 0);                                             \
    return arr->_array[0];                                                  \
}                                                                           \
                                                                            \
type array_back_##type(struct array_##type* arr)                            \
{                                                                           \
    assert(arr->_elements > 0);                                             \
    return arr->_array[arr->_elements - 1];                                 \
}                                                                           \
                                                                            \
type array_get_##type(struct array_##type* arr, size_t index)               \
{                                                                           \
    assert(index < arr->_elements);                                         \
    return arr->_array[index];                                              \
}                                                                           \
                                                                            \
bool array_empty_##type(struct array_##type* arr)                           \
{                                                                           \
    return (arr->_elements == 0);                                           \
}                                                                           \
                                                                            \
size_t array_size_##type(struct array_##type* arr)                          \
{                                                                           \
    return arr->_elements;                                                  \
}                                                                           \
                                                                            \
void array_clear_##type(struct array_##type* arr)                           \
{                                                                           \
    free(arr->_array);                                                      \
    arr->_elements = 0;                                                     \
    arr->_capacity = 0;                                                     \
    arr->_array = calloc(arr->_capacity, arr->_type_size);                  \
}                                                                           \
                                                                            \
void array_reserve_##type(struct array_##type* arr, size_t amount)          \
{                                                                           \
    assert(amount > arr->_capacity);                                        \
                                                                            \
    arr->_capacity = (arr->_capacity > 0) ? arr->_capacity * 2 : 1;         \
    type* tmp = realloc(arr->_array, arr->_type_size * amount);             \
                                                                            \
    assert(tmp != NULL);                                                    \
                                                                            \
    arr->_array = tmp;                                                      \
}                                                                           \
                                                                            \
void array_shrink_##type(struct array_##type* arr)                          \
{                                                                           \
    if (arr->_elements == arr->_capacity)                                   \
        return;                                                             \
                                                                            \
    arr->_capacity = arr->_elements;                                        \
    type* tmp = realloc(arr->_array, arr->_type_size * arr->_capacity);     \
                                                                            \
    assert(tmp != NULL);                                                    \
                                                                            \
    arr->_array = tmp;                                                      \
 }
