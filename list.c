#include <stdio.h>
#include <stdlib.h>
#include <assert.h> // Para assert
#include "list.h"

// Definición de la estructura del Nodo
typedef struct Node Node;

struct Node {
    void * data;
    Node * next;
    Node * prev;
};

// Definición de la estructura de la Lista
// No es necesario el 'typedef List List;' después de esta definición,
// ya que 'typedef struct List List;' se usa antes en list.h (o debería estar allí).
// Si 'typedef struct List List;' no está en list.h antes de las declaraciones de funciones,
// entonces 'struct List' debe usarse en los parámetros de función en list.h.
// Asumiremos que list.h tiene 'typedef struct List List;'
struct List {
    Node * head;
    Node * tail;
    Node * current;
    // int count; // Opcional: para un list_size O(1)
};

// Función para crear un nuevo nodo
Node * createNode(void * data) {
    Node * newNode = (Node *)malloc(sizeof(Node));
    // assert(newNode != NULL); // assert es útil para depuración, pero puede abortar el programa.
                             // Considerar manejo de error si malloc falla en producción.
    if (newNode == NULL) {
        perror("Error: No se pudo asignar memoria para el nodo (createNode)");
        return NULL; // Retornar NULL si malloc falla
    }
    newNode->data = data;
    newNode->prev = NULL;
    newNode->next = NULL;
    return newNode;
}

// Función para crear una nueva lista
List * createList() {
    List* newList = (List *) malloc(sizeof(List));
    // assert(newList != NULL);
    if (newList == NULL) {
        perror("Error: No se pudo asignar memoria para la lista (createList)");
        return NULL; // Retornar NULL si malloc falla
    }
    newList->head = NULL;
    newList->tail = NULL;
    newList->current = NULL;
    // newList->count = 0; // Si se añade 'count' a la struct List
    return newList;
}

// Retorna el dato del primer elemento de la lista y actualiza current
void * firstList(List * list) {
    if (list == NULL || list->head == NULL) {
        if (list) list->current = NULL; // Asegurar que current sea NULL si la lista está vacía
        return NULL;
    }
    list->current = list->head;
    return list->current->data;
}

// Retorna el dato del siguiente elemento de la lista y actualiza current
void * nextList(List * list) {
    if (list == NULL || list->current == NULL || list->current->next == NULL) {
        // Si current está en el último nodo, o no hay current, no hay siguiente.
        // Opcionalmente, se podría poner list->current = NULL aquí si se llega al final.
        return NULL;
    }
    list->current = list->current->next;
    return list->current->data;
}

// Retorna el dato del último elemento de la lista y actualiza current
void * lastList(List * list) {
    if (list == NULL || list->tail == NULL) {
        if (list) list->current = NULL;
        return NULL;
    }
    list->current = list->tail;
    return list->current->data;
}

// Retorna el dato del elemento anterior de la lista y actualiza current
void * prevList(List * list) {
    if (list == NULL || list->current == NULL || list->current->prev == NULL) {
        // Si current está en el primer nodo, o no hay current, no hay anterior.
        // Opcionalmente, se podría poner list->current = NULL aquí si se llega al inicio.
        return NULL;
    }
    list->current = list->current->prev;
    return list->current->data;
}

// Agrega un elemento al inicio de la lista
void pushFront(List * list, void * data) {
    if (list == NULL) return;
    Node * newNode = createNode(data);
    if (newNode == NULL) return; // createNode ya maneja el error de malloc

    newNode->next = list->head;
    if (list->head != NULL) {
        list->head->prev = newNode;
    } else {
        // Si la lista estaba vacía, el nuevo nodo es también la cola
        list->tail = newNode;
    }
    list->head = newNode;
    // list->current = newNode; // Opcional: actualizar current al nuevo nodo
    // if (list->count != NULL) list->count++; // Si se usa 'count'
}

// Agrega un elemento al final de la lista (CORREGIDO)
void pushBack(List * list, void * data) {
    if (list == NULL) return;
    Node * newNode = createNode(data);
    if (newNode == NULL) return; // createNode ya maneja el error de malloc

    if (list->head == NULL) { // Si la lista está vacía
        list->head = newNode;
        list->tail = newNode;
        // list->current = newNode; // Opcional: actualizar current
    } else { // Si la lista no está vacía
        list->tail->next = newNode;
        newNode->prev = list->tail;
        list->tail = newNode;
        // list->current = newNode; // Opcional: actualizar current
    }
    // if (list->count != NULL) list->count++; // Si se usa 'count'
}

// Agrega un elemento después del nodo current
void pushCurrent(List * list, void * data) {
    if (list == NULL || list->current == NULL) {
        // No se puede insertar si no hay lista o no hay un 'current' establecido
        return;
    }
    Node * newNode = createNode(data);
    if (newNode == NULL) return;

    newNode->next = list->current->next;
    newNode->prev = list->current;

    if (list->current->next != NULL) {
        list->current->next->prev = newNode;
    } else {
        // Si current era el último nodo, el nuevo nodo es ahora la cola
        list->tail = newNode;
    }
    list->current->next = newNode;
    // list->current = newNode; // Opcional: mover current al nuevo nodo insertado
    // if (list->count != NULL) list->count++;
}

// Elimina el primer elemento de la lista y retorna su dato
void * popFront(List * list) {
    if (list == NULL || list->head == NULL) return NULL;
    
    // Establecer current al head para que popCurrent lo elimine
    list->current = list->head; 
    return popCurrent(list);
}

// Elimina el último elemento de la lista y retorna su dato
void * popBack(List * list) {
    if (list == NULL || list->tail == NULL) return NULL;

    // Establecer current al tail para que popCurrent lo elimine
    list->current = list->tail;
    return popCurrent(list);
}

// Elimina el elemento current de la lista y retorna su dato
void * popCurrent(List * list) {
    if (list == NULL || list->current == NULL) {
        return NULL;
    }
    
    Node * nodeToRemove = list->current;
    void * data = nodeToRemove->data;

    Node * prevNode = nodeToRemove->prev;
    Node * nextNode = nodeToRemove->next;

    if (prevNode != NULL) {
        prevNode->next = nextNode;
    } else {
        // Se está eliminando la cabeza de la lista
        list->head = nextNode;
    }

    if (nextNode != NULL) {
        nextNode->prev = prevNode;
        list->current = nextNode; // Mover current al siguiente nodo
    } else {
        // Se está eliminando la cola de la lista
        list->tail = prevNode;
        list->current = prevNode; // Mover current al nodo anterior (que ahora es la cola o NULL)
    }

    free(nodeToRemove);
    // if (list->count != NULL) list->count--;
    return data;
}

// Elimina todos los nodos de la lista (pero no los datos que contienen)
void cleanList(List * list) {
    if (list == NULL) return;
    // No es necesario setear current para popFront, ya que popFront lo hace.
    while (list->head != NULL) {
        popFront(list); // popFront actualiza head y maneja la liberación del nodo
    }
    // Después de limpiar, head, tail y current deberían ser NULL.
    // popFront, a través de popCurrent, ya debería dejar head y tail en NULL
    // cuando la lista se vacía. current también se actualiza.
    // list->count = 0; // Si se usa 'count'
}

// Retorna el número de elementos en la lista.
// Nota: Esta implementación es O(n). Para O(1), se necesitaría un miembro 'count' en struct List.
int list_size(List* L) {
    if (L == NULL || L->head == NULL) {
        return 0;
    }
    int count = 0;
    Node* temp = L->head;
    while (temp != NULL) {
        count++;
        temp = temp->next;
    }
    return count;
}

// Las siguientes funciones estaban en tu list.h original pero no en list.c.
// Las he comentado aquí. Si las necesitas, deberás implementarlas.
/*
// Esta función elimina el último elemento de la lista.
void *list_popBack(List *L) {
    // Implementación (sería igual a popBack)
    return popBack(L);
}

// Esta función elimina el elemento actual de la lista.
void *list_popCurrent(List *L) {
    // Implementación (sería igual a popCurrent)
    return popCurrent(L);
}

// Esta función elimina todos los elementos de la lista.
void list_clean(List *L) {
    // Implementación (sería igual a cleanList)
    cleanList(L);
}

// Función para insertar ordenado de acuerdo a la función lower_than
void list_sortedInsert(List *L, void *data,
                       int (*lower_than)(void *data1, void *data2)) {
    // ... Implementación requerida ...
    // Esta es más compleja:
    // 1. Si la lista está vacía o data es menor que head, pushFront.
    // 2. Iterar por la lista con 'current' hasta encontrar el lugar correcto
    //    (donde current->data < data y (current->next == NULL o data < current->next->data)).
    // 3. Usar pushCurrent (o una lógica similar) para insertar después de 'current' o
    //    manejar la inserción antes de 'current->next'.
    // 4. Cuidado con los casos de borde (insertar al final).
    printf("list_sortedInsert no implementado.\n");
}
*/