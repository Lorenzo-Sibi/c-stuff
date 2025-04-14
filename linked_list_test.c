#include "linked_list.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include "int_list.h"

#define MAX_NUM_ITEMS 64

#define TEST_SIZE_1 1000000
#define TEST_SIZE_2 10000000
#define TEST_SIZE_3 100000000

typedef struct TestItem
{
    ListItem item; // Must be first field to allow casting
    int value;
} TestItem;

// ========== Helper functions ==========

TestItem *create_test_item(int value)
{
    TestItem *ti = (TestItem *)malloc(sizeof(TestItem));
    if (!ti)
    {
        perror("Failed to allocate TestItem");
        exit(EXIT_FAILURE);
    }
    ti->value = value;
    return ti;
}

void print_list(LinkedList *list)
{
    ListItem *curr = list->first;
    printf("List (size %d): ", list->size);
    while (curr)
    {
        TestItem *ti = (TestItem *)curr;
        printf("%d -> ", ti->value);
        curr = curr->next;
    }
    printf("NULL\n");
}

// ========== Test Cases ==========

void test_init()
{
    printf("\n=== Testing List_init ===\n");
    LinkedList list;
    List_init(&list);
    assert(list.first == NULL);
    assert(list.last == NULL);
    assert(list.size == 0);
    printf("PASS: List initialization\n");
}

void test_push()
{
    printf("\n=== Testing List_push ===\n");
    LinkedList list;
    List_init(&list);

    TestItem *item1 = create_test_item(10);
    TestItem *item2 = create_test_item(20);
    TestItem *item3 = create_test_item(30);

    List_push(&list, (ListItem *)item1);
    assert(list.first == (ListItem *)item1);
    assert(list.last == (ListItem *)item1);
    assert(list.size == 1);
    assert(item1->item.next == NULL);
    assert(item1->item.prev == NULL);
    printf("PASS: Push to empty list\n");

    List_push(&list, (ListItem *)item2);
    assert(list.first == (ListItem *)item2);
    assert(list.last == (ListItem *)item1);
    assert(list.size == 2);
    assert(item2->item.next == (ListItem *)item1);
    assert(item2->item.prev == NULL);
    assert(item1->item.prev == (ListItem *)item2);
    printf("PASS: Push to non-empty list\n");

    List_push(&list, (ListItem *)item3);
    assert(list.first == (ListItem *)item3);
    assert(list.size == 3);
    print_list(&list);

    List_free(&list);
}

void test_search()
{
    printf("\n=== Testing List_search ===\n");
    LinkedList list;
    List_init(&list);

    TestItem *items[5];
    for (int i = 0; i < 5; i++)
    {
        items[i] = create_test_item(i * 10);
        List_push(&list, (ListItem *)items[i]);
    }

    // Search for existing items
    for (int i = 0; i < 5; i++)
    {
        ListItem *found = List_search(&list, (ListItem *)items[i]);
        assert(found == (ListItem *)items[i]);
    }
    printf("PASS: Search for existing items\n");

    // Search for non-existing item
    TestItem *not_in_list = create_test_item(999);
    ListItem *found = List_search(&list, (ListItem *)not_in_list);
    assert(found == NULL);
    printf("PASS: Search for non-existing item\n");

    free(not_in_list);
    List_free(&list);
}

void test_remove()
{
    printf("\n=== Testing List_remove ===\n");
    LinkedList list;
    List_init(&list);

    TestItem *items[5];
    for (int i = 0; i < 5; i++)
    {
        items[i] = create_test_item(i * 10);
        List_push(&list, (ListItem *)items[i]);
    }

    print_list(&list);

    // Remove from the middle
    printf("Removing middle item (value: %d)\n", items[2]->value);
    ListItem *removed = List_remove(&list, (ListItem *)items[2]);
    assert(removed == (ListItem *)items[2]);
    assert(list.size == 4);
    print_list(&list);

    // Remove first item
    printf("Removing first item (value: %d)\n", items[4]->value);
    removed = List_remove(&list, (ListItem *)items[4]);
    assert(removed == (ListItem *)items[4]);
    assert(list.first == (ListItem *)items[3]);
    assert(list.size == 3);
    print_list(&list);

    // Remove last item
    printf("Removing last item (value: %d)\n", items[0]->value);
    removed = List_remove(&list, (ListItem *)items[0]);
    assert(removed == (ListItem *)items[0]);
    assert(list.last == (ListItem *)items[1]);
    assert(list.size == 2);
    print_list(&list);

    for (int i = 0; i < 5; i++)
    {
        if (i != 0 && i != 2 && i != 4)
        { // Skip already removed items
            List_remove(&list, (ListItem *)items[i]);
        }
        free(items[i]);
    }
    assert(list.size == 0);
    assert(list.first == NULL);
    assert(list.last == NULL);
}

void test_pop()
{
    printf("\n=== Testing List_pop ===\n");
    LinkedList list;
    List_init(&list);

    // Test pop on empty list
    ListItem *popped = List_pop(&list);
    assert(popped == NULL);
    printf("PASS: Pop on empty list returns NULL\n");

    // Push items
    TestItem *items[3];
    for (int i = 0; i < 3; i++)
    {
        items[i] = create_test_item(i * 10);
        List_push(&list, (ListItem *)items[i]);
    }
    print_list(&list);

    // Pop items one by one
    for (int i = 0; i < 3; i++)
    {
        printf("Before pop: size=%d, last=%d\n", list.size, ((TestItem *)list.last)->value);
        popped = List_pop(&list);
        TestItem *ti = (TestItem *)popped;
        printf("Popped value: %d\n", ti->value);
        assert(ti == items[i]);
        printf("After pop: size=%d\n", list.size);
        print_list(&list);
    }

    assert(list.size == 0);
    assert(list.first == NULL);
    assert(list.last == NULL);
    printf("PASS: List is empty after popping all items\n");

    for (int i = 0; i < 3; i++)
        free(items[i]);
}

void performance_test()
{
    printf("\n=== Performance Testing ===\n");
    const long TEST_SIZES[] = {TEST_SIZE_1, TEST_SIZE_2, TEST_SIZE_3};
    const int NUM_SIZES = sizeof(TEST_SIZES) / sizeof(TEST_SIZES[0]);

    for (int s = 0; s < NUM_SIZES; s++)
    {
        int n = TEST_SIZES[s];
        printf("\nTesting with %d elements\n", n);

        LinkedList list;
        List_init(&list);
        TestItem **items = (TestItem **)malloc(n * sizeof(TestItem *));
        if (!items)
        {
            perror("Failed to allocate items array");
            exit(EXIT_FAILURE);
        }

        // Measure push performance
        clock_t start = clock();
        for (int i = 0; i < n; i++)
        {
            items[i] = create_test_item(i);
            List_push(&list, (ListItem *)items[i]);
        }
        clock_t end = clock();
        double push_time = ((double)(end - start)) / CLOCKS_PER_SEC;
        printf("Push time for %d items: %.6f seconds (%.2f ns/item)\n",
               n, push_time, (push_time * 1e9) / n);

        // Measure search performance (best, worst, average case)
        // Best case: first item
        start = clock();
        List_search(&list, (ListItem *)items[n - 1]); // First item due to push order
        end = clock();
        double search_best = ((double)(end - start)) / CLOCKS_PER_SEC;
        printf("Search time (best case): %.6f seconds\n", search_best);

        // Worst case: last item
        start = clock();
        List_search(&list, (ListItem *)items[0]); // Last item due to push order
        end = clock();
        double search_worst = ((double)(end - start)) / CLOCKS_PER_SEC;
        printf("Search time (worst case): %.6f seconds\n", search_worst);

        // Average case: random access
        start = clock();
        for (int i = 0; i < 100; i++)
        {
            int idx = rand() % n;
            List_search(&list, (ListItem *)items[idx]);
        }
        end = clock();
        double search_avg = ((double)(end - start)) / CLOCKS_PER_SEC / 100;
        printf("Search time (average case over 100 searches): %.6f seconds\n", search_avg);

        // Measure pop performance
        start = clock();
        for (int i = 0; i < n; i++)
        {
            List_pop(&list);
        }
        end = clock();
        double pop_time = ((double)(end - start)) / CLOCKS_PER_SEC;
        printf("Pop time for %d items: %.6f seconds (%.2f ns/item)\n",
               n, pop_time, (pop_time * 1e9) / n);

        for (int i = 0; i < n; i++)
        {
            free(items[i]);
        }
        free(items);
    }
}

int main()
{
    srand(time(NULL));

    test_init();
    test_push();
    test_search();
    test_remove();
    test_pop();

    performance_test();

    printf("\nAll tests completed successfully!\n");
    return 0;
}

int prev_main()
{
    LinkedList linked_list;
    List_init(&linked_list);
    for (int i = 0; i < MAX_NUM_ITEMS; ++i)
    {
        IntListItem *new_element = (IntListItem *)
            malloc(sizeof(IntListItem));
        if (!new_element)
        {
            printf("out of memory\n");
            break;
        }
        new_element->list.prev = 0;
        new_element->list.next = 0;
        new_element->info = i;
        int result = List_push(&linked_list, (ListItem *)new_element);
        assert(result);
    }
    IntList_print(&linked_list);

    printf("removing odd elements");
    ListItem *aux = linked_list.first;
    int k = 0;
    while (aux)
    {
        ListItem *item = aux;
        aux = aux->next;
        if (k % 2)
        {
            List_remove(&linked_list, item);
            free(item);
        }
        ++k;
    }
    IntList_print(&linked_list);

    printf("removing from the linked_list, half of the list");
    int size = linked_list.size;
    k = 0;
    while (linked_list.first && k < size / 2)
    {
        ListItem *item = List_remove(&linked_list, linked_list.first);
        assert(item);
        free(item);
        ++k;
    }
    IntList_print(&linked_list);

    printf("removing from the tail the rest of the list, until it has 1 element");
    while (linked_list.first && linked_list.size > 1)
    {
        ListItem *item = List_remove(&linked_list, linked_list.last);
        assert(item);
        free(item);
    }

    IntList_print(&linked_list);

    printf("removing last element");
    ListItem *item = List_remove(&linked_list, linked_list.last);
    assert(item);
    free(item);
    IntList_print(&linked_list);

    List_free(&linked_list);

    return 0;
}