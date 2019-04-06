
#include <check.h>
#include <stdlib.h>
#include <stdint.h>

#include "check_arraylist.h"

int main(void) {
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = arraylist_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
	system("pause");
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}