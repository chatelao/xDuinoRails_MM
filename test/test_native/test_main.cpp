#ifdef NATIVE_TESTING
#include <Arduino.h>
#include <unity.h>

void test_native_environment(void) {
    TEST_ASSERT_EQUAL(true, true);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_native_environment);
    return UNITY_END();
}
#else
// Dummy functions for the embedded environment
void setup() {}
void loop() {}
#endif
