#include <Arduino.h>
#include <unity.h>

// Forward declarations for test functions
void test_cv_manager_get_set(void);
void test_cv_manager_defaults(void);
void test_cv_manager_special(void);

void test_native_environment(void) {
    TEST_ASSERT_EQUAL(true, true);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_native_environment);
    RUN_TEST(test_cv_manager_get_set);
    RUN_TEST(test_cv_manager_defaults);
    RUN_TEST(test_cv_manager_special);
    return UNITY_END();
}
