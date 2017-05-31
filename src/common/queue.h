#ifndef T1_LIB_QUEUE_H
#define T1_LIB_QUEUE_H

/* TODO REVISAR SI ESTA ES LA MEJOR IMPLEMENTACION DE UNA COLA */

#include <stdbool.h>

/* Nodo para la cola */
struct queue_node
{
    /** Contenido de este nodo */
    void* content;
    /** Siguiente nodo de la cola */
    struct queue_node* next;
};

/* Cola que almacena objetos en orden FIFO */
struct queue
{
    /** El proximo nodo a atender */
    struct queue_node* head;

    /** El ultimo nodo que entró */
    struct queue_node* tail;

    /** Cuantos elementos tiene la cola */
    int count;
};
/** Cola que almacena objetos en orden FIFO */
typedef struct queue Queue;

/** Inicializa una cola vacia */
Queue* queue_init     ();
/** Libera los recursos asociados a la cola */
void   queue_destroy  (Queue* queue);
/** Mete un puntero cualquiera a la cola */
void   queue_enqueue  (Queue* queue, void* pointer);
/** Obtiene el siguiente puntero de la cola */
void*  queue_dequeue  (Queue* queue);
/** Inidica si la cola está vacia o no */
bool   queue_is_empty (Queue* queue);
/** Vacía la cola */
void   queue_clear    (Queue* queue);

#endif /* End of include guard: T1_LIB_QUEUE_H */
