// Stack implementation in C based on C++ vector implementation

/*  HOW TO USE:

    Call STACK(type) with the desired type, multiple types can be used.
    Call constructor_stack(type) to define attributes and function pointers.
    Call destructor(stk) in order to clean up.
    If stack goes out of scope without destructor being called, a memory leak will occur.

    example:

    STACK(int)

    int main(void)
    {
        stack_int stk = constructor_stack(int);

        stk.push_int(&stk, 1);
        stk.push_int(&stk, 2);

        destructor(stk);
        
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

// test
// sup

#define constructor_stack(type) {                                                                   \
    ._elements = 0, ._capacity = 0, ._elem_size = sizeof(type), ._array = malloc(sizeof(type) * 0), \
    .push_##type = stack_push_##type,                                                               \
    .pop_##type = stack_pop_##type,                                                                 \
    .top_##type = stack_top_##type,                                                                 \
    .empty_##type = stack_empty_##type, .size_##type = stack_size_##type }

#ifndef destructor
    #define destructor(item) \
        free(item._array);   \
        item._array = NULL;  \
        item._elements = 0;  \
        item._capacity = 0;  \
        item._elem_size = 0;
#endif

#define STACK(type) typedef struct stack_##type                         \
{                                                                       \
    type *_array;                                                       \
    size_t _elements;                                                   \
    size_t _capacity;                                                   \
    size_t _elem_size;                                                  \
    void   (*push_##type)(struct stack_##type*, type);                  \
    void   (*pop_##type)(struct stack_##type*);                         \
    type   (*top_##type)(struct stack_##type*);                         \
    bool   (*empty_##type)(struct stack_##type*);                       \
    size_t (*size_##type)(struct stack_##type*);                        \
} stack_##type;                                                         \
void stack_push_##type(struct stack_##type* stk, type elem)             \
{                                                                       \
    if (stk->_elements >= stk->_capacity)                               \
    {                                                                   \
        size_t _capacity = stk->_capacity;                              \
        type *cpy = malloc(stk->_elem_size * _capacity);                \
        memcpy(cpy, stk->_array, stk->_elem_size * _capacity);          \
        free(stk->_array);                                              \
        stk->_capacity = (stk->_capacity > 0) ? stk->_capacity * 2 : 1; \
        stk->_array = malloc(stk->_elem_size * stk->_capacity);         \
        memcpy(stk->_array, cpy, stk->_elem_size * _capacity);          \
        free(cpy);                                                      \
    }                                                                   \
    stk->_array[stk->_elements] = elem;                                 \
    stk->_elements++;                                                   \
}                                                                       \
void stack_pop_##type(struct stack_##type* stk)                         \
{                                                                       \
    assert(stk->_elements > 0);                                         \
    stk->_elements--;                                                   \
}                                                                       \
type stack_top_##type(struct stack_##type* stk)                         \
{                                                                       \
    assert(stk->_elements > 0);                                         \
    return stk->_array[stk->_elements - 1];                             \
}                                                                       \
bool stack_empty_##type(struct stack_##type* stk)                       \
{                                                                       \
    return (stk->_elements == 0);                                       \
}                                                                       \
size_t stack_size_##type(struct stack_##type* stk)                      \
{                                                                       \
    return stk->_elements;                                              \
}
