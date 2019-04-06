
#include "list.h"

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

struct sortedlist create_sortedlist(uint64_t obj_size, list_comparator comp) {
    struct sortedlist a;
    a.list = create_arraylist(obj_size);
    a.comp = comp;
}

void destroy_sortedlist(struct sortedlist* list) {
    destroy_arraylist(&list->list);
}

static size_t find_insertion_point(struct sortedlist* list, const void* val,
                                   bool exact) {
    size_t curr;
    size_t i = list->list.len;
    size_t l = 0;
    size_t r = list->list.len;

    while (l <= r) {
        curr = l + (r - l) / 2;
        int c = list->comp(
            val, (const void*) (&list->list.val + curr * list->list.obj_size));
        if (c == 0) {
            return curr;
        }
        if (c == -1) {
            l = curr + 1;
        } else {
            r = curr - 1;
        }
    }
    if (exact) {
        return -1;
    } else {
        return curr;
    }
}

void push_sortedlist(struct sortedlist* list, const void* val) {
    insert_arraylist(&list->list, val, find_insertion_point(list, val, false));
}

size_t find_sortedlist(struct sortedlist* list, const void* val) {
    return find_insertion_point(list, val, true);
}

void add_range(struct rangeset* set, struct range range) {}
