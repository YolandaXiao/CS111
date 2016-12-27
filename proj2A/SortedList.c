#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "SortedList.h"

void SortedList_insert(SortedList_t *list, SortedListElement_t *element)
{
    if(list==NULL || element==NULL)
        return;
    SortedListElement_t* current = list;
    while (current->next != list) {
    	if(strcmp(current->next->key,element->key)<0)
    		break;
    	current = current->next;
    }

    if(opt_yield & INSERT_YIELD)
    	pthread_yield();

    element->next = current->next;
    current->next = element;
    element->prev = current;
    (element->next)->prev = element; 
}

int SortedList_delete( SortedListElement_t *element)
{
    if(element==NULL)
        return 1;
    else if (element->next == NULL || element->next->prev != element)
        return 1;
    else if (element->prev == NULL || element->prev->next != element)
        return 1;
    if(opt_yield & DELETE_YIELD)
        pthread_yield();
    (element->next)->prev = element->prev;
    (element->prev)->next = element->next;

    return 0;
}



SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key)
{
    if(list==NULL || key==NULL)
        return NULL;
    SortedListElement_t* current = list;
    while (current->next != list) {
    	if(opt_yield & LOOKUP_YIELD)
    		pthread_yield();
    	if(current->next && strcmp(current->next->key,key)==0)
    		return current->next;
        else if(current->next)
    	   current = current->next;
        else
            break;
    }

    return NULL;
}

//question: current->next != NULL
int SortedList_length(SortedList_t *list)
{
    SortedListElement_t* current = list;
    int count = 0;

    while (current->next != list) {
    	count++;
    	if(opt_yield & LOOKUP_YIELD)
    		pthread_yield();
    	if(current->next->prev != current || current->prev->next != current)
	  return -1;
    	current = current->next;
    }
    return count;
}
