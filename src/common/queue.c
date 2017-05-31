#include <stdlib.h>
#include "queue.h"

/** Inicializa una cola vacia */
Queue* queue_init()
{
    /* Solicita memoria para la cola */
    Queue* queue = malloc(sizeof(Queue));
    /* Creamos un nodo vacio donde se pondrá el proximo elemento que venga */
    struct queue_node* node = malloc(sizeof(struct queue_node));
    /* Este nodo no apunta a nada */
    node -> next = NULL;
    /* El proximo elemento que venga se pondrá en este nodo */
    queue -> head = node;
    /* Y será el primero en salir cuando llegue el momento */
    queue -> tail = node;
    /* Inicialmente no tiene ningun elemento */
    queue -> count = 0;
    /* Retornamos la cola lista */
    return queue;
}

/** Libera los recursos asociados a la cola */
void queue_destroy(Queue* queue)
{
    /* Mientras la cola no esté vacia */
    while(queue -> head != queue -> tail)
    {
        /* Vamos destruyendo los nodos y avanzando */
        struct queue_node* aux = queue -> head;
        queue -> head = aux -> next;
        free(aux);
    }
    /* Mientras la ultima celda exista */
    while(queue -> tail)
    {
        /* La eliminamos y pasamos a la siguiente */
        struct queue_node* aux = queue -> tail -> next;
        free(queue -> tail);
        queue -> tail = aux;
    }

    /* Libera la memoria de la cola */
    free(queue);
}

/** Mete un puntero cualquiera a la cola */
void queue_enqueue(Queue* queue, void* pointer)
{
    /* Marcamos que tenemos un elemento más */
    queue -> count++;
    /* Si no existía ya */
    if(!queue -> tail -> next)
    {
        /* Creamos un nuevo nodo vacio para el proximo elemento */
        queue -> tail -> next = malloc(sizeof(struct queue_node));
        /* Este nodo no apunta a nada */
        queue -> tail -> next -> next = NULL;
    }

    /* Metemos el puntero en el nodo actual */
    queue -> tail -> content = pointer;
    /* Apuntamos al nuevo nodo vacio */
    queue -> tail = queue -> tail -> next;
}

/** Obtiene el siguiente puntero de la cola */
void* queue_dequeue(Queue* queue)
{
    /* Marcamos que tenemos un elemento menos */
    queue -> count--;
    /* Obtenemos el puntero almacenado */
    void* pointer = queue -> head -> content;
    /* El nodo en el que estaba guardado ahora será destruido */
    struct queue_node* node = queue -> head;
    /* Pero antes nos aseguramos de apuntar al nodo siguiente */
    queue -> head = queue -> head -> next;
    /* Ahora si, liberamos la memoria del nodo */
    free(node);
    /* Retornamos el puntero que estaba guardado */
    return pointer;
}

/** Inidica si la cola está vacia o no */
bool queue_is_empty(Queue* queue)
{
    /* La cola está vacia si ambos punteros estan en el mismo nodo */
    return (queue -> head == queue -> tail);
}

/** Vacía la cola */
void queue_clear(Queue* queue)
{
    // while(queue_is_empty(queue)) queue_dequeue(queue);
    /* En realidad no la vacia, solo mueve tail a la posicion inicial */
    queue -> tail = queue -> head;
    /* Y marca como que ahora hay 0 elementos */
    queue -> count = 0;
}
