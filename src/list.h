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

void push_sortedlist(struct arraylist* list, const void* val,
                     list_comparator comp);

int find_sortedlist(struct arraylist* list, const void* val,
                    list_comparator comp);

void remove_sortedlist(struct arraylist* list, size_t index);

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
    struct arraylist ranges;
};

struct range create_range(uint64_t start, uint64_t len);

void destroy_range(struct range*);

bool in_range(struct range*, uint64_t addr);

bool is_overlapping_range(struct range*, struct range*);

/**
 * Merges two overlapping ranges. The first parameter will contain the merged
 * range, with the combined callbacks inside, and the second range will be freed
 */
void merge_overlapping_range(struct range*, struct range*);

struct rangeset create_rangeset();

void add_range(struct rangeset* set, struct range range);

bool in_rangeset(struct rangeset* set, uint64_t val);

/**
 *
 * @param set
 * @param addr Address to return all callback ids
 * @return
 */
struct range_callback* fetch_callback_rangeset(struct rangeset* set,
                                               uint64_t addr);

#endif
