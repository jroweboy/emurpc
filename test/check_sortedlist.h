#ifndef CHECK_EMURPC_SORTEDLIST_H
#define CHECK_EMURPC_SORTEDLIST_H

#include "list.h"
#include <check.h>
#include <stdlib.h>

struct sortedlist* sortedlist;

int compare_int(const void* left, const void* right) {
    return *((const int*) right) - *((const int*) left);
}

void sortedlist_setup(void) {
    sortedlist = malloc(sizeof(struct sortedlist));
    *sortedlist = create_sortedlist(sizeof(int), compare_int);
}

void sortedlist_teardown(void) {
    destroy_sortedlist(sortedlist);
    free(sortedlist);
}

// Core

START_TEST(test_sortedlist_insert) {
    int l[11] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    for (int i = 0; i < 11; ++i) {
        push_sortedlist(sortedlist, &l[i]);
    }
    for (int i = 0; i < 11; ++i) {
        ck_assert_int_eq(((int*) sortedlist->list.val)[i], i);
    }
}
END_TEST

START_TEST(test_sortedlist_insert_reverse) {
    int l[11] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    for (int i = 10; i >= 0; --i) {
        push_sortedlist(sortedlist, &l[i]);
    }
    for (int i = 0; i < 11; ++i) {
        ck_assert_int_eq(((int*) sortedlist->list.val)[i], i);
    }
}
END_TEST

START_TEST(test_sortedlist_find) {
    int l[11] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    for (int i = 0; i < 11; ++i) {
        push_sortedlist(sortedlist, &l[i]);
    }
    size_t index = find_sortedlist(sortedlist, &l[5]);
    ck_assert_int_eq(((int*) sortedlist->list.val)[index], 5);
}
END_TEST

// Limits

START_TEST(test_sortedlist_insert_remove) {
    int l = 5;
    push_sortedlist(sortedlist, &l);
    remove_sortedlist(sortedlist, 0);
    ck_assert_int_eq(sortedlist->list.len, 0);
}
END_TEST

Suite* sortedlist_suite() {
    Suite* s;
    TCase* tc_core;
    TCase* tc_limits;

    s = suite_create("SortedList");

    /* Core test case */
    tc_core = tcase_create("Core");

    tcase_add_checked_fixture(tc_core, sortedlist_setup, sortedlist_teardown);
    tcase_add_test(tc_core, test_sortedlist_insert);
    tcase_add_test(tc_core, test_sortedlist_insert_reverse);
    tcase_add_test(tc_core, test_sortedlist_find);
    suite_add_tcase(s, tc_core);

    /* Limits test case */
    tc_limits = tcase_create("Limits");
    tcase_add_test(tc_core, test_sortedlist_insert_remove);
    suite_add_tcase(s, tc_limits);

    return s;
}
#endif
