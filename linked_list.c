#include "linked_list.h"

void List_init(LinkedList *l)
{
    l->first = NULL;
    l->last = NULL;
    l->size = 0;
}

void List_free(LinkedList *l)
{
    ListItem *iter = l->first;
    while (iter)
    {
        free(List_remove(l, iter));
        iter = l->first;
    }
}

ListItem *List_search(const LinkedList *l, ListItem *li)
{
    ListItem *iter = l->first;
    while (iter)
    {
        if (iter == li)
            return li;
        iter = iter->next;
    }
    return NULL;
}

int List_push(LinkedList *l, ListItem *li)
{
    ListItem *aux = l->first;
    if (!aux)
    { // empty list
        l->first = li;
        l->last = li;
        l->size++;
        li->next = NULL;
        li->prev = NULL;
        return 1;
    }
    // assert that aux->next == NULL
    aux->prev = li;
    li->prev = NULL;
    li->next = aux;

    l->first = li;
    l->size++;
    return 1;
}

ListItem *List_remove(LinkedList *l, ListItem *li)
{
#ifdef _LIST_DEBUG_
    // we check that the element is in the list
    ListItem *instance = List_find(head, item);
    assert(instance);
#endif
    ListItem *prev = li->prev;
    ListItem *next = li->next;

    if (prev)
        prev->next = next;
    if (next)
        next->prev = prev;

    if (li == l->first)
        l->first = next;
    if (li == l->last)
        l->last = prev;
    l->size--;
    li->next = li->prev = NULL;
    return li;
}

// Remove the last element in the list
ListItem *List_pop(LinkedList *l)
{
    return List_remove(l, l->last);
}