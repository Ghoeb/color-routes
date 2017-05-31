#ifndef T1_GENSOL_SOLVER_H
#define T1_GENSOL_SOLVER_H

#include "city.h"
#include "queue.h"

#define GENERATOR          10
#define OPTIMIZATION_NONE   0
#define OPTIMIZATION_GREEDY 1
#define OPTIMIZATION_BEST   2

/* Representa una decision */
struct choice
{
    /** El nodo que tomó la decision */
    Building*  choser;
    /** La pareja que escogió */
    Building*  chosee;
    /** Los nodos que fueron pintados debido a esto */
    Queue* affected;
};
/** Representa una decisión */
typedef struct choice Choice;

/* Un stack especial para las decisiones, que tienen cantidad definida */
struct choice_stack
{
    /** Un arreglo de desiciones */
    Choice* choices;

    /** Apunta a la siguiente desicion */
    size_t  pointer;

    /** El tamaño del stack */
    size_t  size;
};
/** Un stack especial para las decisiones, que tienen cantidad definida */
typedef struct choice_stack CStack;

/** Crea un nuevo stack vacio con el tamaño especificado */
CStack* choice_stack_init     (size_t size  );
/** Libera los recursos asociados al stack */
void    choice_stack_destroy  (CStack* stack);
/** Copia la decision a la celda correspondiente */
void    choice_stack_push     (CStack* stack, Choice choice);
/** Saca y entrega la ultima decision efectuada */
Choice  choice_stack_pop      (CStack* stack);
/** Indica si el stack está vacio o no */
bool    choice_stack_is_empty (CStack* stack);
/** Entrega la ultima desicion efectuada, sin sacarla */
Choice  choice_stack_peek     (CStack* stack);

/* Inicializa las funciones a usar segun el nivel de optimizacion */
void solver_init_x(int optimization);

/* Resuelve el grafo y deja las decisiones (Choice) tomadas en el stack */
bool solver_solve(Layout* graph, CStack* steps);

#endif /* end of include guard: T1_GENSOL_SOLVER_H */
