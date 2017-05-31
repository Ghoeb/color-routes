#ifndef T1_SOL_SOLVER_N
#define T1_SOL_SOLVER_N

#include "../common/city.h"

/** Para estadísticas: la cantidad de veces que el algoritmo se devolvió */
unsigned int undo_count;

/** Indica que debe imprimir paso a paso el estado del algoritmo */
bool step;

/** Representa una decisión */
typedef struct
{
    /** El cliente que tomo la decision */
    Client* choser;
    /** El cliente que fue escogido */
    Client* chosen;
    /** El cliente que cambió de color, en caso de existir */
    Client* tinted;
} Choice;

/** Resuelve el problema o retorna false si no puede */
bool solve(Layout* layout, Choice* steps);

#endif /* end of include guard: T1_SOL_SOLVER_E */
