#include "backtracking.h"
#include <stdio.h>
#include "../common/stack.h"
#include "../common/queue.h"
#include "../common/heap.h"

bool solver;

/** La cantidad de cadenas que tiene el puzzle */
int chain_count = 0;

/** La cantidad de veces que se ha hecho UNDO */
uint32_t undo_count = 0;

/*#########################################################################*/
/*                          Funciones Asignables                           */
/*#########################################################################*/

/** Resuelve el grafo y computa los pasos necesarios para resolverlo */
bool  (* sol_solve)           (Layout*, CStack*);

/** Calcula cual es el siguiente nodo a asignar */
Client* (* sol_next)            (Layout*);

/** Calcula las opciones para el nodo escogido */
bool  (* sol_options)         (Building*);

/** Pinta un nodo de un color y registra todos los que cambian por eso */
bool  (* sol_paint_observe)   (Client*, Color, Queue*);

/*#########################################################################*/
/*                                 Common                                  */
/*#########################################################################*/

/* Indica si un par de nodos son pareja válida */
bool sol_is_valid_pair(Client* node1, Client* node2)
{
    if(node1 -> color == node2 -> color) return true;
    if(!node1 -> color || !node2 -> color) return true;
    return false;
}

/* Indica que emparejar estos dos nodos es una jugada legal */
bool sol_is_legal(Client* node1, Client* node2)
{
    return node1 != node2 &&
           !city_client_is_taken(node2) &&
           sol_is_valid_pair(node1, node2);
}

/* Lleva a cabo una decision y la guarda en el stack */
bool sol_choice_make(Client* node1, Client* node2, CStack* steps)
{
    city_client_link(node1, node2);
    if(solver)
    {
        city_client_link_print(node1, node2);
    }

    Choice choice = steps -> choices[steps -> pointer];
    choice.choser = node1;
    choice.chosee = node2;
    queue_clear(choice.affected);
    choice_stack_push(steps, choice);

    /* Si son de distinto color*/
    if(node1 -> color != node2 -> color)
    {
        if(node1 -> color)
        {
            return sol_paint_observe(node2, node1 -> color, choice.affected);
        }
        else
        {
            return sol_paint_observe(node1, node2 -> color, choice.affected);
        }
    }
    /* Si son del mismo color (y tienen color) */
    else if(node1 -> color)
    {
        /* Descontamos la cantidad  de cadenas que hay */
        chain_count--;
        /* TODO ESTO ES POWER OPTIMIZATION ELIMINAR PARA BASIC PUZZLES */
        /* Si no nos quedan cadenas no se puede completar el puzzle */
        if(steps -> pointer < steps -> size && !chain_count) return false;
    }
    return true;
}

/** Deshace la ultima decision efectuada */
Client* sol_choice_undo(CStack* steps)
{
    undo_count++;

    Choice choice = choice_stack_pop(steps);

    city_client_link_undo(choice.choser, choice.chosee);
    if(solver)
    {
        city_client_link_undo_print(choice.choser, choice.chosee);
    }

    while(!queue_is_empty(choice.affected))
    {
        Client* affected = queue_dequeue(choice.affected);
        affected -> color = none;
    }

    if(choice.choser -> color &&
       choice.choser -> color == choice.chosee -> color)
    {
        chain_count++;
    }
    return choice.choser;
}



/*#########################################################################*/
/*                                Randomness                               */
/*#########################################################################*/

/* Calcula las opciones para un nodo dado */
bool sol_options_random(Client* node)
{
    Heap* options = node -> options;

    heap_clear(options);

    Zone* cell = node -> zone;

    for(int i = 0; i < cell -> building_count; i++)
    {
        Client* val = cell -> buildings[i];
        if(sol_is_legal(node, val))
        {
            heap_insert(options, val, rand());
        }
    }

    return !heap_is_empty(options);
}

/*#########################################################################*/
/*                                  Basic                                  */
/*#########################################################################*/

/* TODO quizas un node-queue no es lo más apropiado */
/* Pinta un nodo de un color y registra los nodos que cambian debido a eso */
bool sol_paint_just_watch(Client* target, Color color, Queue* queue)
{
    bool ret = true;
    if(!target -> color)
    {
        target -> color = color;
        if(queue) queue_enqueue(queue, target);
        for(int i = 0; i  < target -> link_count; i++)
        {
            Client* victim = target -> linked[i];
            ret = ret && sol_paint_just_watch(victim, color, queue);
        }
    }
    return ret;
}

/* Encuentra el siguiente nodo sin asignar. Toma el primero que ve */
Client* sol_next_first(Layout* graph)
{
    for(int i = 0; i < graph -> zone_count; i++)
    {
        for(int j = 0; j < graph -> zones[i] -> building_count; j++)
        {
            if(!city_client_is_taken(graph -> zones[i] -> buildings[j]))
            {
                return graph -> zones[i] -> buildings[j];
            }
        }
    }
    for(int i = 0; i < graph -> zone_count; i++)
    {
        for(int j = 0; j < graph -> zones[i] -> building_count; j++)
        {
            if(city_client_is_blank(graph -> zones[i] -> buildings[j]))
            {
                return graph -> zones[i] -> buildings[j];
            }
        }
    }
    return NULL;
}

/* Calcula las opciones para un nodo dado */
bool sol_options_first(Client* node)
{
    Heap* options = node -> options;

    heap_clear(options);

    for(int i = 0; i < node -> zone -> building_count; i++)
    {
        Client* val = node -> zone -> buildings[i];
        if(sol_is_legal(node, val))
        {
            heap_insert(options, val, 0);
        }
    }

    return !heap_is_empty(options);
}

/* Usa backtracking para encontrar una asignacion valida de parejas */
bool sol_solver_recursive(Layout* graph, CStack* steps)
{
    Client* next = sol_next(graph);

    if(!next) return true;

    /* Calcula las opciones para el nodo escogido */
    sol_options(next);

    while(!heap_is_empty(next -> options))
    {
        Client* val = heap_extract(next -> options);

        /* Si la eleccion no dejo al puzzle en un estado inresolvible */
        if(sol_choice_make(next, val, steps))
        {
            /* Intentamos resolver el puzzle en este nuevo estado */
            if(sol_solver_recursive(graph, steps))
            {
                return true;
            }
        }

        sol_choice_undo(steps);
    }

    return false;
}


/*#########################################################################*/
/*                                 Greedy                                  */
/*#########################################################################*/

/* Encuentra el siguiente nodo sin asignar. Toma el que le parece mejor */
Client* sol_next_greedy(Layout* graph)
{
    Client* selected = NULL;

    for(int i = 0; i < graph -> zone_count; i++)
    {
        for(int j = 0; j < graph -> zones[i] -> building_count; j++)
        {
            Client* challenger = graph -> zones[i] -> buildings[j];

            /* Si está listo definitivamente no nos interesa */
            if(city_client_is_ready(challenger)) continue;

            /* Si no tenemos ninguno, esta es la mejor opcion */
            if(!selected) selected = challenger;

            /* Priorizar los que tienen color, y no estan conectados */
            if(challenger -> color)
            {
                return challenger;
            }
            /* Priorizar los que no tienen vecinos */
            else if(!city_client_is_taken(challenger) && city_client_is_taken(selected))
            {
                return challenger;
            }
        }
    }

    return selected;
}

/** Calcula las opciones para un nodo, priorizando las del mismo color */
bool sol_options_greedy(Client* node)
{
    Heap* options = node -> options;

    heap_clear(options);

    Zone* cell = node -> zone;

    for(int i = 0; i < cell -> building_count; i++)
    {
        Client* val = cell -> buildings[i];
        if(sol_is_legal(node, val))
        {
            /* Si es que el nodo tiene color y ambos son del mismo color */
            if(node -> color && node -> color == val -> color)
            {
                heap_insert(options, val, 1);
            }
            else
            {
                heap_insert(options, val, 0);
            }
        }
    }

    return !heap_is_empty(options);
}

/* Efectúa backtracking sobre el grafo para encontrar la solucion */
bool sol_solver_iterative(Layout* graph, CStack* steps)
{
    do
    {
        /* Obtenemos el siguiente nodo a asignar */
        Client* next = sol_next(graph);

        /* Si no quedan nodos por asignar, ya terminamos */
        if(!next) return true;

        /* Si el nodo no tiene opciones o la opcion tomada es invalida  */
        if(!sol_options(next) ||
           !sol_choice_make(next, heap_extract(next -> options), steps))
        {
            do
            {
                /* Recuperamos el nodo que tomo la ultima desicion */
                do
                {
                    /* Si no nos queda a donde volver */
                    if(choice_stack_is_empty(steps))
                    {
                        /* El puzzle no se puede resolver */
                        return false;
                    }
                    /* Volvemos al nodo anterior */
                    next = sol_choice_undo(steps);

            /* Si a ese nodo no le quedan opciones, volvemos a retroceder */
            } while(heap_is_empty(next -> options));

            /* Retrocedemos hasta poder tomar una opcion valida */
            } while(!sol_choice_make(next,
                                     heap_extract(next -> options),
                                     steps));
        }

    /* Cuando el stack esté vacio, significa que ya probamos todo */
    } while(!choice_stack_is_empty(steps));

    /* Si ya probamos todo, entonces no era resolvible */
    return false;
}

/*#########################################################################*/
/*                               Thoughtful                                */
/*#########################################################################*/

/* Matriz que indica el orden en que se deben probar los nodos */
int score_matrix[4][8] =
{
    {10000},
    {9000, 5000, 1000},
    {8000, 6000, 4000, 2000, 1000},
    {7000, 6000, 5000, 4000, 3000, 2000, 1000}
};

/* Calcula el puntaje de un nodo. Mientras mayor sea, antes se evaluará */
int sol_node_score(Client* node)
{
    /* Solo si no ha sido emparejado tiene puntaje */
    if(city_client_is_taken(node)) return 0;

    /* El hecho de que sea no emparejado le da alta prioridad */
    int score = 100000;

    /* Luego le asignamos puntaje segun cuantas opciones tiene */
    int total_options = node -> zone -> building_count / 2 - 1;
    int actual_options = node -> options -> count - 1;
    /* Entre 1000 y 10000 */
    score += score_matrix[total_options][actual_options];

    /* El hecho de que tenga color tambien le da prioridad */
    if(node -> color) score += 500;

    /* Entrega el puntaje final del nodo */
    return score;
}

/* Encuentra el siguiente nodo sin asignar. Toma el con mejor puntaje */
Client* sol_next_smarter(Layout* graph)
{
    Client* selected = NULL;

    int score = 0;

    for(int i = 0; i < graph -> zone_count; i++)
    {
        for(int j = 0; j < graph -> zones[i] -> building_count; j++)
        {
            Client* challenger = graph -> zones[i] -> buildings[j];

            /* Si está listo definitivamente no nos interesa */
            if(city_client_is_ready(challenger)) continue;

            /* Si no tenemos ninguno, esta es la mejor opcion */
            if(!selected)
            {
                selected = challenger;
                score = sol_node_score(selected);
            }
            else
            {
                sol_options(challenger);

                int challenscore = sol_node_score(challenger);
                if(challenscore > score)
                {
                    selected = challenger;
                    score = challenscore;
                }
            }
        }
    }

    return selected;
}


/* Revisa que la disposicion de colores de la celda sea resolvible */
bool sol_cell_is_solvable(Zone* cell)
{
    /* Contamos los colores de los nodos que quedan por emparejar */
    uint8_t color_count[8] = {0,0,0,0,0,0,0,0};

    for(int i = 0; i < cell -> building_count; i++)
    {
        if(!city_client_is_taken(cell -> buildings[i]))
        {
            color_count[cell -> buildings[i] -> color]++;
        }
    }
    for(int i = 7; i > 0; i--)
    {
        for(int j = 0; j < color_count[i]; j++)
        {
            color_count[i]--;
            if(color_count[i] > 0)
            {
                color_count[i]--;
            }
            else if(color_count[0] > 0)
            {
                color_count[0]--;
            }
            else
            {
                return false;
            }
        }
    }
    return true;
}

/* TODO quizas un node-queue no es lo más apropiado */
/* Pinta un nodo de un color y registra los nodos que cambian debido a eso */
bool sol_paint_check_zones(Client* target, Color color, Queue* queue)
{
    if(!target -> color)
    {
        target -> color = color;
        if(queue) queue_enqueue(queue, target);
        /* Si es que la celda quedó inresolvible entonces lo avisamos */
        if(!sol_cell_is_solvable(target -> zone))
        {
            return false;
        }
        for(int i = 0; i  < target -> link_count; i++)
        {
            if(!sol_paint_check_zones(target -> linked[i], color, queue))
            {
                return false;
            }
        }
    }
    return true;
}

/*#########################################################################*/
/*                                 Hipster                                 */
/*#########################################################################*/


/*#########################################################################*/
/*                                 Public                                  */
/*#########################################################################*/

void solver_init_x(int optimization)
{
    switch (optimization)
    {
        case GENERATOR:
        sol_solve = sol_solver_iterative;
        sol_next = sol_next_smarter;
        sol_options = sol_options_random;
        sol_paint_observe = sol_paint_check_zones;
        solver = false;
        break;
        case OPTIMIZATION_NONE:
        sol_solve = sol_solver_recursive;
        sol_next = sol_next_first;
        sol_options = sol_options_first;
        sol_paint_observe = sol_paint_just_watch;
        break;
        case OPTIMIZATION_GREEDY:
        sol_solve = sol_solver_iterative;
        sol_next = sol_next_smarter;
        sol_options = sol_options_greedy;
        sol_paint_observe = sol_paint_check_zones;
        break;
        default: /* TODO SE CAE  */
        sol_solve = sol_solver_iterative;
        sol_next = sol_next_smarter;
        sol_options = sol_options_random;
        sol_paint_observe = sol_paint_check_zones;
        solver = false;

        break;
    }
}

/* TODO DARSE CUENTA CUANDO UN NODO QUEDA SIN OPCIONES */
/* TODO DARSE CUENTA CUANDO UNA CELDA QUEDA INRESOLVIBLE */
/* TODO DARSE CUENTA CUANDO YA NO QUEDAN CADENAS */
bool solver_solve(Layout* graph, CStack* steps)
{
    /* Inicializa las colas que se usaran para almacenar a los candidatos */
    for(int i = 0; i < graph -> zone_count; i++)
    {
        Zone* cell = graph -> zones[i];
        for(int j = 0; j < cell -> building_count; j++)
        {
            Client* node = cell -> buildings[j];
            /* Inicializamos un heap para los candidatos posibles */
            node -> options = heap_init(cell -> building_count - 1);
        }
    }



    for(int i = 0; i < graph -> core_count; i++)
    {
        Client* core = graph -> cores[i] -> buildings[0];
        chain_count += core -> link_count;
    }
    chain_count /= 2;



    bool ret = sol_solve(graph, steps);

    fprintf(stderr, "Resuelto volviendo %d veces\n", undo_count);

    /* Libera la memoria de las colas */
    for(int i = 0; i < graph -> zone_count; i++)
    {
        for(int j = 0; j < graph -> zones[i] -> building_count; j++)
        {
            heap_destroy(graph -> zones[i] -> buildings[j] -> options);
        }
    }
    return ret;
}
