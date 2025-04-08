#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

typedef struct ListItem
{
    struct ListItem *prev;
    struct ListItem *next;
} ListItem;

typedef struct LinkedList
{
    ListItem *first;
    ListItem *last;
    int size;
} LinkedList;

void List_init(LinkedList *l);
void List_free(LinkedList *l);

int List_push(LinkedList *l, ListItem *li);
int List_insert(LinkedList *l);
ListItem *List_remove(LinkedList *l, ListItem *li);
ListItem *List_pop(LinkedList *l);

ListItem *List_search(const LinkedList *l, ListItem *li);