#ifndef LIST_h
#define LIST_h

typedef struct List List;

List * createList(void);

void * firstList(List * list);

void * nextList(List * list);

void * lastList(List * list);

void * prevList(List * list);

void pushFront(List * list, void * data);

void pushBack(List * list, void * data);

void pushCurrent(List * list, void * data);

void * popFront(List * list);

void * popBack(List * list);

void * popCurrent(List * list);

void cleanList(List * list);



// Esta función elimina el último elemento de la lista.
void *list_popBack(List *L);

// Esta función elimina el elemento actual de la lista.
void *list_popCurrent(List *L);

// Esta función elimina todos los elementos de la lista.
void list_clean(List *L);

// Función para insertar ordenado de acuerdo a la función lower_than
void list_sortedInsert(List *L, void *data,
                       int (*lower_than)(void *data1, void *data2));

int list_size(List* L);

#endif /* LIST_h */
