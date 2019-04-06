#include "emurpc/types.h"

#ifndef EMURPC_LIST_H
#define EMURPC_LIST_H

typedef int (*list_comparator)(const void* left, const void* right);

/// Auto resizing array list

struct arraylist {
    uint8_t* val;
    uint64_t len;
    uint64_t capacity;
    uint64_t obj_size;
};

struct arraylist create_arraylist(uint64_t obj_size);

bool insert_arraylist(struct arraylist* list, const void* val, size_t index);

void push_arraylist(struct arraylist* list, const void* val);

void remove_arraylist(struct arraylist* list, size_t index);

void pop_arraylist(struct arraylist* list);

size_t find_first_arraylist(struct arraylist* list, const void* val,
                            list_comparator comp);

void destroy_arraylist(struct arraylist* list);

/// Sorted List

struct sortedlist {
    struct arraylist list;
    list_comparator comp;
};

struct sortedlist create_sortedlist(uint64_t obj_size, list_comparator comp);

void destroy_sortedlist(struct sortedlist* list);

void push_sortedlist(struct sortedlist* list, const void* val);

int find_sortedlist(struct sortedlist* list, const void* val);

void remove_sortedlist(struct sortedlist* list, size_t index);

/// Ranges

struct range_callback {
    uint64_t start;
    uint64_t end;
    uint32_t id;
};

struct range {
    uint64_t start;
    uint64_t end;
    struct arraylist callbacks;
};

struct rangeset {
    struct sortedlist ranges;
};

void add_range(struct rangeset* set, struct range range);

#endif
