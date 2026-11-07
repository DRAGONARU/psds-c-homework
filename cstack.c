#include "cstack.h"

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define UNUSED(VAR) (void)(VAR)

struct Node
{
    struct Node* prev;
    unsigned int size;
    unsigned char data[];
};

struct StackEntry
{
    struct Node* top;
    int reserved;
    unsigned int count;
};

enum
{
    STACK_CAPACITY = 32
};
static struct StackEntry stack_table[STACK_CAPACITY];

hstack_t stack_new(void)
{
    for (int idx = 0; idx < STACK_CAPACITY; idx++)
    {
        if (stack_table[idx].reserved == 0)
        {
            stack_table[idx].reserved = 1;
            stack_table[idx].top = NULL;
            stack_table[idx].count = 0;
            return idx;
        }
    }
    return -1;
}

void stack_free(const hstack_t hstack)
{
    if (!stack_valid_handler(hstack))
    {
        return;
    }
    struct Node* current = stack_table[hstack].top;
    while (current != NULL)
    {
        struct Node* previous = current->prev;
        free(current);
        current = previous;
    }
    stack_table[hstack].top = NULL;
    stack_table[hstack].count = 0;
    stack_table[hstack].reserved = 0;
}

void stack_free_all(void)
{
    for (int idx = 0; idx < STACK_CAPACITY; idx++)
    {
        stack_free(idx);
    }
}

int stack_valid_handler(const hstack_t hstack)
{
    if (hstack < 0 || hstack >= STACK_CAPACITY)
    {
        return 0;
    }
    return stack_table[hstack].reserved != 0;
}

unsigned int stack_size(const hstack_t hstack)
{
    if (stack_valid_handler(hstack))
    {
        return stack_table[hstack].count;
    }
    return 0;
}

static int
push_internal(hstack_t hstack, const void* data_in, unsigned int size)
{
    if (!stack_valid_handler(hstack) || data_in == NULL || size == 0)
    {
        return 0;
    }
    struct StackEntry* entry = &stack_table[hstack];
    if (entry->count == UINT_MAX)
    {
        return 0;
    }
    size_t data_size = size;
    if (data_size > SIZE_MAX - sizeof(struct Node))
    {
        return 0;
    }
    struct Node* node = malloc(sizeof *node + data_size);
    if (node == NULL)
    {
        return 0;
    }
    node->prev = entry->top;
    node->size = size;
    memcpy(node->data, data_in, size);
    entry->top = node;
    entry->count++;
    return 1;
}

void
stack_push(const hstack_t hstack, const void* data_in, const unsigned int size)
{
    (void)push_internal(hstack, data_in, size);
}

unsigned int
stack_pop(const hstack_t hstack, void* data_out, const unsigned int size)
{
    if (!stack_valid_handler(hstack))
    {
        return 0;
    }
    if (stack_table[hstack].top == NULL)
    {
        return 0;
    }
    if (data_out == NULL)
    {
        return 0;
    }
    if (size == 0)
    {
        return 0;
    }
    if (size < stack_table[hstack].top->size)
    {
        return 0;
    }
    struct Node* node = stack_table[hstack].top;
    unsigned int node_size = node->size;
    memcpy(data_out, node->data, node->size);
    stack_table[hstack].top = node->prev;
    stack_table[hstack].count--;
    free(node);
    return node_size;
}

unsigned stack_top_size(const hstack_t hstack)
{
    if (!stack_valid_handler(hstack))
    {
        return 0;
    }
    if (stack_table[hstack].top == NULL)
    {
        return 0;
    }
    return stack_table[hstack].top->size;
}

hstack_t stack_get_all(void)
{
    hstack_t result = stack_new();

    if (result == -1)
    {
        return -1;
    }

    for (hstack_t handle = 0; handle < STACK_CAPACITY; handle++)
    {
        if (handle == result || !stack_valid_handler(handle))
        {
            continue;
        }

        if (!push_internal(result, &handle, sizeof(hstack_t)))
        {
            stack_free(result);
            return -1;
        }
    }

    return result;
}
