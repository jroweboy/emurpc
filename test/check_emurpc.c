
#include <check.h>
#include <stdint.h>
#include <stdlib.h>

#include "check_arraylist.h"
#include "check_sortedlist.h"

int main(void) {
    int number_failed;
    Suite* arraylist_s;
    Suite* sortedlist_s;
    SRunner* sr;

    arraylist_s = arraylist_suite();
    sortedlist_s = sortedlist_suite();
    sr = srunner_create(arraylist_s);
    srunner_add_suite(sr, sortedlist_s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    system("pause");
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
