// tests/test_main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "../jsonp.c" // Assumes your parser header is here

// --- Minimal Test Framework ---
int tests_run = 0;
int tests_passed = 0;
int tests_failed = 0;

#define ASSERT_TRUE(cond) do { \
    if (!(cond)) { \
        printf("\033[0;31m[FAILED]\033[0m %s:%d: Assertion '%s' failed.\n", __FILE__, __LINE__, #cond); \
        tests_failed++; \
        return; \
    } \
} while(0)

#define ASSERT_STR_EQ(expected, actual) do { \
    const char* e = (expected); const char* a = (actual); \
    if (strcmp(e, a) != 0) { \
        printf("\033[0;31m[FAILED]\033[0m %s:%d: Expected \"%s\", got \"%s\"\n", __FILE__, __LINE__, e, a); \
        tests_failed++; \
        return; \
    } \
} while(0)

#define RUN_TEST(test_func) do { \
    printf("[RUNNING] %s...\n", #test_func); \
    int fails_before = tests_failed; \
    test_func(); \
    if (tests_failed == fails_before) { \
        printf("\033[0;32m[PASSED]\033[0m %s\n", #test_func); \
        tests_passed++; \
    } \
    tests_run++; \
} while(0)

// --- Test Cases ---

void test_parse_simple_object() {
    const char *json = "{\"key\": \"value\"}";
    JsonNode *root = json_parse(json, strlen(json));
    
    ASSERT_TRUE(root != NULL);
    ASSERT_TRUE(root->type == JSON_OBJECT);
    
    JsonNode *val = json_get_member(root, "key");
    ASSERT_TRUE(val != NULL);
    ASSERT_TRUE(val->type == JSON_STRING);
    ASSERT_STR_EQ("value", json_as_string(val));
    
    json_delete(root);
}

void test_parse_numbers() {
    const char *json = "{\"int\": 123, \"float\": 45.67}";
    JsonNode *root = json_parse(json, strlen(json));
    
    ASSERT_TRUE(root != NULL);
    
    JsonNode *n1 = json_get_member(root, "int");
    ASSERT_TRUE(n1 != NULL && n1->type == JSON_NUMBER);
    ASSERT_TRUE(json_as_number(n1) == 123.0);
    
    JsonNode *n2 = json_get_member(root, "float");
    ASSERT_TRUE(n2 != NULL && n2->type == JSON_NUMBER);
    // Note: Float comparison usually needs epsilon, simplified here
    ASSERT_TRUE(json_as_number(n2) > 45.6 && json_as_number(n2) < 45.7);
    
    json_delete(root);
}

void test_parse_nested_object() {
    const char *json = "{\"meta\": {\"count\": 1}}";
    JsonNode *root = json_parse(json, strlen(json));
    
    ASSERT_TRUE(root != NULL);
    
    JsonNode *meta = json_get_member(root, "meta");
    ASSERT_TRUE(meta != NULL && meta->type == JSON_OBJECT);
    
    JsonNode *count = json_get_member(meta, "count");
    ASSERT_TRUE(count != NULL);
    ASSERT_TRUE(json_as_number(count) == 1.0);
    
    json_delete(root);
}

void test_parse_array() {
    const char *json = "{\"list\": [1, 2, 3]}";
    JsonNode *root = json_parse(json, strlen(json));
    
    ASSERT_TRUE(root != NULL);
    
    JsonNode *list = json_get_member(root, "list");
    ASSERT_TRUE(list != NULL && list->type == JSON_ARRAY);
    
    JsonNode *item = list->child; // First item
    ASSERT_TRUE(item != NULL && item->type == JSON_NUMBER);
    ASSERT_TRUE(json_as_number(item) == 1.0);
    
    item = item->next; // Second item
    ASSERT_TRUE(item != NULL);
    ASSERT_TRUE(json_as_number(item) == 2.0);
    
    json_delete(root);
}

void test_invalid_json() {
    const char *bad_json = "{\"key\": "; // Incomplete
    JsonNode *root = json_parse(bad_json, strlen(bad_json));
    ASSERT_TRUE(root == NULL); // Parser should return NULL on error
}

int main() {
    printf("=== Starting JSON Parser Test Suite ===\n");
    
    RUN_TEST(test_parse_simple_object);
    RUN_TEST(test_parse_numbers);
    RUN_TEST(test_parse_nested_object);
    RUN_TEST(test_parse_array);
    RUN_TEST(test_invalid_json);
    
    printf("\n=== Test Summary ===\n");
    printf("Run: %d, Passed: %d, Failed: %d\n", tests_run, tests_passed, tests_failed);
    
    return (tests_failed == 0) ? 0 : 1;
}