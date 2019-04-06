
#include "list.h"
#include <string.h>

struct arraylist create_arraylist(uint64_t obj_size) {
    struct arraylist a;
    a.capacity = 10;
    a.obj_size = obj_size;
    a.val = calloc(a.capacity, a.capacity * a.obj_size);
    a.len = 0;
    return a;
}

static bool resize_arraylist_if_needed(struct arraylist* list) {
    if (list->len < list->capacity) {
        return false;
    }
    list->capacity *= 2;
    list->val = realloc(list->val, list->obj_size * list->capacity);
    return !!list->val;
}

bool insert_arraylist(struct arraylist* list, const void* val, size_t index) {
    if (0 > index || index > list->len) {
        return false;
    }
    resize_arraylist_if_needed(list);
    uint8_t* src = list->val + index * list->obj_size;
    // Move all elements after the index over to make room unless we are
    // inserting at the end of the list
    if (list->len != index) {
        memmove(src + list->obj_size, src,
                (list->len - index) * list->obj_size);
    }
    memcpy(src, (const uint8_t*) val, list->obj_size);
    list->len++;
    return true;
}

void push_arraylist(struct arraylist* list, const void* val) {
    insert_arraylist(list, val, list->len);
}

void remove_arraylist(struct arraylist* list, size_t index) {
    if (0 > index || list->len <= index) {
        return;
    }
    uint8_t* dst = list->val + index * list->obj_size;
    if (list->len != index) {
        memmove(dst, dst + list->obj_size,
                (list->len - index) * list->obj_size);
    }
    list->len--;
}

void pop_arraylist(struct arraylist* list) { remove_arraylist(list, 0); }

size_t find_first_arraylist(struct arraylist* list, const void* val,
                            list_comparator comp) {
    size_t i;
    for (i = 0; i < list->len; ++i) {
        if (comp(val, (const void*) (list->val + i * list->obj_size)) == 0) {
            return i;
        }
    }
    return -1;
}

void destroy_arraylist(struct arraylist* list) {
    if (list->val) {
        free(list->val);
    }
    list->capacity = 0;
    list->len = 0;
}

static int find_insertion_point(struct arraylist* list, const void* val,
                                list_comparator comp, bool exact) {
    int l = 0;
    int r = list->len - 1;
    int curr = l + (r - l) / 2;
    while (l <= r) {
        int c = comp(val, (const void*) (list->val + curr * list->obj_size));
        if (c == 0) {
            return curr;
        }
        if (c < 0) {
            l = curr + 1;
        } else {
            r = curr - 1;
        }
        curr = l + (r - l) / 2;
    }
    if (exact) {
        return -1;
    } else {
        return curr;
    }
}

void push_sortedlist(struct arraylist* list, const void* val,
                     list_comparator comp) {
    insert_arraylist(list, val, find_insertion_point(list, val, comp, false));
}

int find_sortedlist(struct arraylist* list, const void* val,
                    list_comparator comp) {
    return find_insertion_point(list, val, comp, true);
}

void remove_sortedlist(struct arraylist* list, size_t index) {
    remove_arraylist(list, index);
}

struct range create_range(uint64_t start, uint64_t len);

void add_range(struct rangeset* set, struct range range) {}
