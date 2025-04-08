#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "linked_list.h"
#include "int_list.h"

#define MAX_NUM_ITEMS 64

int main(int argc, char **argv)
{
    // we populate the list, by inserting MAX_NUM_ITEMS
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
}