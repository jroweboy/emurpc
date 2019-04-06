#ifndef CHECK_EMURPC_ARRAYLIST_H
#define CHECK_EMURPC_ARRAYLIST_H

#include "list.h"
#include <check.h>
#include <stdlib.h>

struct arraylist* arraylist;

void arraylist_setup(void) {
    arraylist = malloc(sizeof(struct arraylist));
    *arraylist = create_arraylist(sizeof(int));
}

void arraylist_teardown(void) {
    destroy_arraylist(arraylist);
    free(arraylist);
}

// Core

START_TEST(test_arraylist_resize) {
    int l[11] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    for (int i = 0; i < 11; ++i) {
        push_arraylist(arraylist, &l[i]);
    }
    ck_assert_int_eq(arraylist->capacity, 20);
}
END_TEST

START_TEST(test_arraylist_insert) {
    int l[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    for (int i = 0; i < 10; ++i) {
        push_arraylist(arraylist, &l[i]);
    }
    ck_assert_int_eq(((int*) arraylist->val)[4], 4);
    ck_assert_int_eq(((int*) arraylist->val)[5], 5);
    insert_arraylist(arraylist, &l[9], 5);
    ck_assert_int_eq(((int*) arraylist->val)[4], 4);
    ck_assert_int_eq(((int*) arraylist->val)[5], 9);
}
END_TEST

START_TEST(test_arraylist_remove) {
    int l[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    for (int i = 0; i < 10; ++i) {
        push_arraylist(arraylist, &l[i]);
    }
    ck_assert_int_eq(arraylist->len, 10);
    remove_arraylist(arraylist, 0);
    ck_assert_int_eq(arraylist->len, 9);
    ck_assert_int_eq(((int*) arraylist->val)[0], 1);
}
END_TEST

// Limits

START_TEST(test_arraylist_insert_start) {
    int l[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    for (int i = 0; i < 10; ++i) {
        push_arraylist(arraylist, &l[i]);
    }
    ck_assert_int_eq(((int*) arraylist->val)[4], 4);
    ck_assert_int_eq(((int*) arraylist->val)[5], 5);
    insert_arraylist(arraylist, &l[9], 5);
    ck_assert_int_eq(((int*) arraylist->val)[4], 4);
    ck_assert_int_eq(((int*) arraylist->val)[5], 9);
}
END_TEST

START_TEST(test_arraylist_insert_end) {
    int l[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    for (int i = 0; i < 10; ++i) {
        push_arraylist(arraylist, &l[i]);
    }
    ck_assert_int_eq(((int*) arraylist->val)[4], 4);
    ck_assert_int_eq(((int*) arraylist->val)[5], 5);
    insert_arraylist(arraylist, &l[1], 10);
    ck_assert_int_eq(((int*) arraylist->val)[4], 4);
    ck_assert_int_eq(((int*) arraylist->val)[10], 1);
}
END_TEST

START_TEST(test_arraylist_insert_empty) {
    int i = 5;
    ck_assert_int_eq(arraylist->len, 0);
    insert_arraylist(arraylist, &i, 0);
    ck_assert_int_eq(((int*) arraylist->val)[0], 5);
}
END_TEST

START_TEST(test_arraylist_remove_start) {
	int i = 5;
    ck_assert_int_eq(arraylist->len, 0);
    push_arraylist(arraylist, &i);
    ck_assert_int_eq(((int*) arraylist->val)[0], 5);
	pop_arraylist(arraylist);
    ck_assert_int_eq(arraylist->len, 0);
}
END_TEST

START_TEST(test_arraylist_remove_end) {
    int l[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    for (int i = 0; i < 10; ++i) {
        push_arraylist(arraylist, &l[i]);
    }
    ck_assert_int_eq(arraylist->len, 10);
	remove_arraylist(arraylist, 9);
    ck_assert_int_eq(arraylist->len, 9);
}
END_TEST

START_TEST(test_arraylist_remove_empty) {
    ck_assert_int_eq(arraylist->len, 0);
	pop_arraylist(arraylist);
    ck_assert_int_eq(arraylist->len, 0);
}
END_TEST

Suite* arraylist_suite() {
    Suite* s;
    TCase* tc_core;
    TCase* tc_limits;

    s = suite_create("Arraylist");

    /* Core test case */
    tc_core = tcase_create("Core");

    tcase_add_checked_fixture(tc_core, arraylist_setup, arraylist_teardown);
    tcase_add_test(tc_core, test_arraylist_resize);
    tcase_add_test(tc_core, test_arraylist_insert);
    tcase_add_test(tc_core, test_arraylist_remove);
    suite_add_tcase(s, tc_core);

    /* Limits test case */
    tc_limits = tcase_create("Limits");
    tcase_add_test(tc_core, test_arraylist_insert_start);
    tcase_add_test(tc_core, test_arraylist_insert_end);
    tcase_add_test(tc_core, test_arraylist_insert_empty);
    tcase_add_test(tc_core, test_arraylist_remove_start);
    tcase_add_test(tc_core, test_arraylist_remove_end);
    tcase_add_test(tc_core, test_arraylist_remove_empty);
    suite_add_tcase(s, tc_limits);

    return s;
}

#endif CHECK_EMURPC_ARRAYLIST_H
