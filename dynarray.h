// Dynamic array implementation in C based on C++ vector implementation

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

    Do not manually modify: _elements, _capacity, _elem_size, or _array.
    Use function pointers to do so.

*/

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include <stdbool.h>

#define constructor_array(type) {                                                                                     \
    ._elements = 0, ._capacity = 0, ._elem_size = sizeof(type), ._array = malloc(sizeof(type) * 0),                   \
    .push_back_##type = array_push_##type, .insert_##type = array_insert_##type,                                      \
    .pop_back_##type = array_pop_back_##type, .erase_##type = array_erase_##type, .clear_##type = array_clear_##type, \
    .front_##type = array_front_##type, .back_##type = array_back_##type, .get_##type = array_get_##type,             \
    .empty_##type = array_empty_##type, .size_##type = array_size_##type,                                             \
    .reserve_##type = array_reserve_##type, .shrink_##type = array_shrink_##type }

#ifndef destructor
    #define destructor(item) \
        free(item._array);   \
        item._array = NULL;  \
        item._elements = 0;  \
        item._capacity = 0;  \
        item._elem_size = 0;
#endif

#define ARRAY(type) typedef struct array_##type                            \
{                                                                          \
    type *_array;                                                          \
    size_t _elements;                                                      \
    size_t _capacity;                                                      \
    size_t _elem_size;                                                     \
    void   (*push_back_##type)(struct array_##type*, type);                \
    void   (*insert_##type)(struct array_##type*, type, size_t);           \
    void   (*pop_back_##type)(struct array_##type*);                       \
    size_t (*erase_##type)(struct array_##type*, size_t);                  \
    type   (*front_##type)(struct array_##type*);                          \
    type   (*back_##type)(struct array_##type*);                           \
    type   (*get_##type)(struct array_##type*, size_t);                    \
    bool   (*empty_##type)(struct array_##type*);                          \
    size_t (*size_##type)(struct array_##type*);                           \
    void   (*clear_##type)(struct array_##type*);                          \
    void   (*reserve_##type)(struct array_##type*, size_t);                \
    void   (*shrink_##type)(struct array_##type*);                         \
} array_##type;                                                            \
void array_push_##type(struct array_##type* arr, type elem)                \
{                                                                          \
    if (arr->_elements >= arr->_capacity)                                  \
    {                                                                      \
        size_t _capacity = arr->_capacity;                                 \
        type *cpy = malloc(arr->_elem_size * _capacity);                   \
        memcpy(cpy, arr->_array, arr->_elem_size * _capacity);             \
        free(arr->_array);                                                 \
        arr->_capacity = (arr->_capacity > 0) ? arr->_capacity * 2 : 1;    \
        arr->_array = malloc(arr->_elem_size * arr->_capacity);            \
        memcpy(arr->_array, cpy, arr->_elem_size * _capacity);             \
        free(cpy);                                                         \
    }                                                                      \
    arr->_array[arr->_elements] = elem;                                    \
    arr->_elements++;                                                      \
}                                                                          \
void array_insert##type(struct array_##type* arr, type elem, size_t index) \
{                                                                          \
    if (arr->_elements >= arr->_capacity)                                  \
    {                                                                      \
        size_t _capacity = arr->_capacity;                                 \
        type *cpy = malloc(arr->_elem_size * _capacity);                   \
        memcpy(cpy, arr->_array, arr->_elem_size * _capacity);             \
        free(arr->_array);                                                 \
        arr->_capacity = (arr->_capacity > 0) ? arr->_capacity * 2 : 1;    \
        arr->_array = malloc(arr->_elem_size * arr->_capacity);            \
        memcpy(arr->_array, cpy, arr->_elem_size * _capacity);             \
        free(cpy);                                                         \
    }                                                                      \
    assert(index <= arr->_elements);                                       \
    if (index == arr->_elements)                                           \
    {                                                                      \
        arr->_array[arr->_elements] = elem;                                \
        arr->_elements++;                                                  \
    }                                                                      \
    else                                                                   \
    {                                                                      \
        type *source = &arr->_array[index];                                \
        type *destination = &arr->_array[index + 1];                       \
        size_t amount = (arr->_elements - index) * arr->_elem_size;        \
        memmove(destination, source, amount);                              \
        arr->_array[index] = elem;                                         \
        arr->_elements++;                                                  \
    }                                                                      \
}                                                                          \
void array_pop_##type(struct array_##type* arr)                            \
{                                                                          \
    assert(arr->_elements > 0);                                            \
    arr->_elements--;                                                      \
}                                                                          \
size_t array_erase_##type(struct array_##type* arr, size_t index)          \
{                                                                          \
    assert(index <= arr->_elements);                                       \
    if (index == arr->_elements - 1)                                       \
    {                                                                      \
        arr->_elements--;                                                  \
        return index - 1;                                                  \
    }                                                                      \
    else                                                                   \
    {                                                                      \
        type *source = &arr->_array[index + 1];                            \
        type *destination = &arr->_array[index];                           \
        size_t amount = (arr->_elements - index) * arr->_elem_size;        \
        memmove(destination, source, amount);                              \
        arr->_elements--;                                                  \
        return index - 1;                                                  \
    }                                                                      \
}                                                                          \
type array_front_##type(struct array_##type* arr)                          \
{                                                                          \
    assert(arr->_elements > 0);                                            \
    return arr->_array[0];                                                 \
}                                                                          \
type array_back_##type(struct array_##type* arr)                           \
{                                                                          \
    assert(arr->_elements > 0);                                            \
    return arr->_array[arr->_elements - 1];                                \
}                                                                          \
type array_get_##type(struct array_##type* arr, size_t index)              \
{                                                                          \
    assert(index < arr->_elements);                                        \
    return arr->_array[index];                                             \
}                                                                          \
bool array_empty_##type(struct array_##type* arr)                          \
{                                                                          \
    return (arr->_elements == 0);                                          \
}                                                                          \
size_t array_size_##type(struct array_##type* arr)                         \
{                                                                          \
    return arr->_elements;                                                 \
}                                                                          \
void array_clear_##type(struct array_##type* arr)                          \
{                                                                          \
    free(arr->_array);                                                     \
    arr->_elements = 0;                                                    \
    arr->_capacity = 0;                                                    \
    arr->_array = malloc(arr->_elem_size * arr->_capacity);                \
}                                                                          \
void array_reserve_##type(struct array_##type* arr, size_t amount)         \
{                                                                          \
    assert(amount > arr->_capacity);                                       \
    type *cpy = malloc(arr->_elem_size * arr->_elements);                  \
    memcpy(cpy, arr->_array, arr->_elem_size * arr->_elements);            \
    free(arr->_array);                                                     \
    arr->_array = malloc(arr->_elem_size * amount);                        \
    memcpy(arr->_array, cpy, arr->_elem_size * arr->_elements);            \
    free(cpy);                                                             \
    arr->_capacity = amount;                                               \
}                                                                          \
void array_shrink_##type(struct array_##type* arr)                         \
{                                                                          \
    if (arr->_elements == arr->_capacity) return;                          \
    type *cpy = malloc(arr->_elem_size * arr->_elements);                  \
    memcpy(cpy, arr->_array, arr->_elem_size * arr->_elements);            \
    free(arr->_array);                                                     \
    arr->_array = malloc(arr->_elem_size * arr->_elements);                \
    memcpy(arr->_array, cpy, arr->_elem_size * arr->_elements);            \
    free(cpy);                                                             \
    arr->_capacity = arr->_elements;                                       \
}
