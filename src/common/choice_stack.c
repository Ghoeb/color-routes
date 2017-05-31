#include "backtracking.h"

/** Crea un nuevo stack vacio con el tamaño especificado */
CStack* choice_stack_init     (size_t size  )
{
    CStack* stack = malloc(sizeof(CStack));
    stack -> pointer = 0;
    stack -> choices = malloc(sizeof(Choice) * size);
    for(int i = 0; i < size; i++)
    {
        stack -> choices[i].affected = queue_init();
    }
    stack -> size = size;
    return stack;
}
/** Libera los recursos asociados al stack */
void    choice_stack_destroy  (CStack* stack)
{
    for(int i = 0; i < stack -> size; i++)
    {
        queue_destroy(stack -> choices[i].affected);
    }
    free(stack -> choices);
    free(stack);
}
/** Copia la decision a la celda correspondiente */
void    choice_stack_push     (CStack* stack, Choice choice)
{
    stack -> choices[stack -> pointer++] = choice;
}
/** Obtiene la ultima decision efectuada */
Choice  choice_stack_pop      (CStack* stack)
{
    return stack -> choices[--stack -> pointer];
}
/** Indica si el stack está vacio o no */
bool    choice_stack_is_empty (CStack* stack)
{
    return !stack -> pointer;
}

/** Entrega la ultima desicion efectuada, sin sacarla */
Choice  choice_stack_peek     (CStack* stack)
{
    return stack -> choices[stack -> pointer - 1];
}
