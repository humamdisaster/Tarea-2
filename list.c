#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "list.h"

typedef struct Node Node;

struct Node {
    void * data;
    Node * next;
    Node * prev;
};

struct List {
    Node * head;
    Node * tail;
    Node * current;
};

typedef List List;

Node * createNode(void * data) {
    Node * new = (Node *)malloc(sizeof(Node));
    assert(new != NULL);
    new->data = data;
    new->prev = NULL;
    new->next = NULL;
    return new;
}

List * createList() {
    List* newLista = (List *) malloc(sizeof(List));
    assert(newLista != NULL);
    newLista->head = NULL;
    newLista->tail = NULL;
    newLista->current = NULL; 
    return newLista;
}

void * firstList(List * list) {
    if (list == NULL || list->head == NULL) return NULL;
    list->current = list->head;

    return list->current->data; //esto para retornar el dato del primer nodo 
}

void * nextList(List * list) {
    if (list == NULL || list->current == NULL || list->current->next == NULL)
        return NULL;
    
    list->current = list->current->next;
    return list->current->data;
}

void * lastList(List * list) {
    if (list == NULL || list->tail == NULL)
        return NULL;
    
    list->current = list->tail;
    return list->current->data;
}

void * prevList(List * list) {
    if (list == NULL || list->current == NULL || list->current->prev == NULL)
        return NULL;
    
    list->current = list->current->prev;
    return list->current->data;
}

void pushFront(List * list, void * data) {
    if (list == NULL) return;
    Node * new = (Node *)malloc(sizeof(Node));
    if (new == NULL) return; //Verifica si se asigna correctamente la memoria

    new->data = data;
    new->prev = NULL;
    new->next = list->head;
    if (list->head != NULL) list->head->prev = new;

    else list->tail = new; //si la lista esta vacia, el nuevo es tambien el tail
    list->head = new;
}

void pushBack(List * list, void * data) {
    list->current = list->tail;
    pushCurrent(list,data);
}

void pushCurrent(List * list, void * data) {
    if (list == NULL || list->current == NULL)
        return;
    Node * new = (Node *)malloc(sizeof(Node));
    if (new == NULL)
        return;
    
    new->data = data;
    new->next = list->current->next;
    new->prev = list->current;
    if (list->current->next != NULL)
        list->current->next->prev = new;
    else list->tail = new; //si current es el ultimo nodo, se actualiza tail

    list->current->next = new;
}

void * popFront(List * list) {
    list->current = list->head;
    return popCurrent(list);
}

void * popBack(List * list) {
    list->current = list->tail;
    return popCurrent(list);
}

void * popCurrent(List * list) {
    if (list == NULL || list->current == NULL)
        return NULL;
    
    Node * aux = list->current;
    void * data = aux->data;
    if (aux->prev != NULL) aux->prev->next = aux->next;
    else list->head = aux->next; //al eliminar el primer nodo, actualiz head

    if (aux->next != NULL)
    {
        aux->next->prev = aux->prev;
        list->current = aux->next; //apunta al current del siguiente nodo
    }
    else
    {
        list->tail = aux->prev; //si al eliminar el ultimo nodo, actualizamos tail
        list->current = aux->prev; //apunta al current del anterior nodo
    }
    free(aux);
    return data;
}

void cleanList(List * list) {
    while (list->head != NULL) {
        popFront(list);
    }
}
