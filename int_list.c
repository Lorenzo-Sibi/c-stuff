#include <stdio.h>
#include "int_list.h"

void IntList_print(LinkedList *l)
{
    ListItem *aux = l->first;
    printf("[");
    while (aux)
    {
        IntListItem *element = (IntListItem *)aux;
        printf("%d ", element->info);
        aux = aux->next;
    }
    printf("]\n");
}
