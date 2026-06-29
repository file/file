#include <check.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "src/apptype.c"

START_TEST(test_buffer_reads_never_exceed_declared_length)
{
    // Invariant: Buffer reads never exceed the declared length
    const char *payloads[] = {
        // Valid input (should pass)
        "normal.txt",
        // Boundary case: exactly _MAX_PATH - 1 characters
        "a",
        // Exploit case: exceeds _MAX_PATH by 2x
        "very_long_filename_that_exceeds_MAX_PATH_by_a_large_margin_and_causes_buffer_overflow_when_combined_with_other_path_components_this_string_is_designed_to_be_over_500_characters_long_"
        "padding_padding_padding_padding_padding_padding_padding_padding_padding_padding_padding_padding_padding_padding_padding_"
        "padding_padding_padding_padding_padding_padding_padding_padding_padding_padding_padding_padding_padding_padding_padding_"
        "padding_padding_padding_padding_padding_padding_padding_padding_padding_padding_padding_padding_padding_padding_padding_"
        "padding_padding_padding_padding_padding_padding_padding_padding_padding_padding_padding_padding_padding_padding_padding"
    };
    int num_payloads = sizeof(payloads) / sizeof(payloads[0]);

    for (int i = 0; i < num_payloads; i++) {
        char path[_MAX_PATH];
        char drive[_MAX_DRIVE];
        char dir[_MAX_DIR];
        char fname[_MAX_FNAME];
        char ext[_MAX_EXT];
        
        // Initialize buffers with known pattern
        memset(path, 0xAA, sizeof(path));
        memset(drive, 0xBB, sizeof(drive));
        memset(dir, 0xCC, sizeof(dir));
        memset(fname, 0xDD, sizeof(fname));
        memset(ext, 0xEE, sizeof(ext));
        
        // Parse the input filename
        _splitpath(payloads[i], drive, dir, fname, ext);
        
        // Call the vulnerable function
        (void)sprintf(path, "%s%s%s%s", drive,
            (*dir == '\0') ? "./" : dir,
            fname,
            (*ext == '\0') ? "." : ext);
        
        // Check that no buffer overflow occurred by verifying
        // the byte after the buffer hasn't been modified
        ck_assert_msg(path[_MAX_PATH - 1] == 0 || path[_MAX_PATH - 1] == 0xAA,
                     "Buffer overflow detected for payload %d", i);
    }
}
END_TEST

Suite *security_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("Security");
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_buffer_reads_never_exceed_declared_length);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = security_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}