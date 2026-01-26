#include <unity.h>

void setUp(void) {
    // set up tasks if needed
}

void tearDown(void) {
    // clean up tasks if needed
}

void test_native_environment(void) {
    TEST_ASSERT_EQUAL(1, 1);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_native_environment);
    return UNITY_END();
}
